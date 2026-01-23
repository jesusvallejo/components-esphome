#include "rf_cc1101.h"

#ifdef USE_ESP_IDF
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "freertos/task.h"
#define GPIO_READ(pin) gpio_get_level(static_cast<gpio_num_t>(pin))
#define GPIO_MODE_INPUT_SETUP(pin) gpio_set_direction(static_cast<gpio_num_t>(pin), GPIO_MODE_INPUT)
#define DELAY_MS(ms) vTaskDelay(pdMS_TO_TICKS(ms))
#define GET_MILLIS() (xTaskGetTickCount() * portTICK_PERIOD_MS)
#else
#define GPIO_READ(pin) digitalRead(pin)
#define GPIO_MODE_INPUT_SETUP(pin) pinMode(pin, INPUT)
#define DELAY_MS(ms) delay(ms)
#define GET_MILLIS() millis()
#endif

namespace esphome {
namespace wmbus {

  static const char *TAG = "rxLoop";

  bool RxLoop::init(uint8_t mosi, uint8_t miso, uint8_t clk, uint8_t cs,
                    uint8_t gdo0, uint8_t gdo2, float freq, bool syncMode) {
    bool retVal = false;
    this->syncMode = syncMode;
    this->gdo0 = gdo0;
    this->gdo2 = gdo2;
    GPIO_MODE_INPUT_SETUP(this->gdo0);
    GPIO_MODE_INPUT_SETUP(this->gdo2);

    // Initialize native CC1101 driver
    if (!cc1101.init_spi(mosi, miso, clk, cs)) {
      ESP_LOGE(TAG, "Failed to initialize SPI for CC1101");
      return false;
    }

    if (!cc1101.init()) {
      ESP_LOGE(TAG, "CC1101 initialization failed!");
      return false;
    }

    // Write T-Mode RF settings
    for (uint8_t i = 0; i < TMODE_RF_SETTINGS_LEN; i++) {
      cc1101.write_reg(TMODE_RF_SETTINGS_BYTES[i << 1],
                       TMODE_RF_SETTINGS_BYTES[(i << 1) + 1]);
    }

    // Calculate and set frequency
    uint32_t freq_reg = uint32_t(freq * 65536 / 26);
    uint8_t freq2 = (freq_reg >> 16) & 0xFF;
    uint8_t freq1 = (freq_reg >> 8) & 0xFF;
    uint8_t freq0 = freq_reg & 0xFF;

    ESP_LOGD(TAG, "Set CC1101 frequency to %3.3fMHz [%02X %02X %02X]",
             freq/1.0, freq2, freq1, freq0);
    cc1101.write_reg(CC1101_FREQ2, freq2);
    cc1101.write_reg(CC1101_FREQ1, freq1);
    cc1101.write_reg(CC1101_FREQ0, freq0);

    cc1101.strobe(CC1101_SCAL);

    uint8_t cc1101Version = cc1101.get_version();

    if ((cc1101Version != 0) && (cc1101Version != 255)) {
      retVal = true;
      ESP_LOGD(TAG, "CC1101 version '%d'", cc1101Version);
      cc1101.set_rx();
      ESP_LOGD(TAG, "CC1101 initialized");
      DELAY_MS(4);
    }
    else {
      ESP_LOGE(TAG, "CC1101 initialization FAILED!");
    }

    return retVal;
  }

  bool RxLoop::task() {
    do {
      switch (rxLoop.state) {
        case INIT_RX:
          start();
          return false;

        // RX active, waiting for SYNC
        case WAIT_FOR_SYNC:
          if (GPIO_READ(this->gdo2)) { // assert when SYNC detected
            rxLoop.state = WAIT_FOR_DATA;
            sync_time_ = GET_MILLIS();
          }
          break;

        // waiting for enough data in Rx FIFO buffer
        case WAIT_FOR_DATA:
          if (GPIO_READ(this->gdo0)) { // assert when Rx FIFO buffer threshold reached
            uint8_t preamble[2];
            // Read the 3 first bytes,
            cc1101.read_burst(CC1101_RXFIFO, rxLoop.pByteIndex, 3);
            rxLoop.bytesRx = 3;
            const uint8_t *currentByte = rxLoop.pByteIndex;
            // Mode C
            if (*currentByte == WMBUS_MODE_C_PREAMBLE) {
              currentByte++;
              data_in.mode = 'C';
              // Block A
              if (*currentByte == WMBUS_BLOCK_A_PREAMBLE) {
                currentByte++;
                rxLoop.lengthField = *currentByte;
                rxLoop.length = 2 + packetSize(rxLoop.lengthField);
                data_in.block = 'A';
              }
              // Block B
              else if (*currentByte == WMBUS_BLOCK_B_PREAMBLE) {
                currentByte++;
                rxLoop.lengthField = *currentByte;
                rxLoop.length = 2 + 1 + rxLoop.lengthField;
                data_in.block = 'B';
              }
              // Unknown type, reinit loop
              else {
                rxLoop.state = INIT_RX;
                return false;
              }
              // don't include C "preamble"
              *(rxLoop.pByteIndex) = rxLoop.lengthField;
              rxLoop.pByteIndex += 1;
            }
            // Mode T Block A
            else if (decode3OutOf6(rxLoop.pByteIndex, preamble)) {
              rxLoop.lengthField  = preamble[0];
              data_in.lengthField = rxLoop.lengthField;
              rxLoop.length  = byteSize(packetSize(rxLoop.lengthField));
              data_in.mode   = 'T';
              data_in.block  = 'A';
              rxLoop.pByteIndex += 3;
            }
            // Unknown mode, reinit loop
            else {
              rxLoop.state = INIT_RX;
              return false;
            }

            rxLoop.bytesLeft = rxLoop.length - 3;

            if (rxLoop.length < MAX_FIXED_LENGTH) {
              // Set CC1101 into length mode
              cc1101.write_reg(CC1101_PKTLEN, (uint8_t)rxLoop.length);
              cc1101.write_reg(CC1101_PKTCTRL0, FIXED_PACKET_LENGTH);
              rxLoop.cc1101Mode = FIXED;
            }
            else {
              // Set CC1101 into infinite mode
              cc1101.write_reg(CC1101_PKTLEN, (uint8_t)(rxLoop.length%MAX_FIXED_LENGTH));
            }

            rxLoop.state = READ_DATA;
            max_wait_time_ += extra_time_;

            cc1101.write_reg(CC1101_FIFOTHR, RX_FIFO_THRESHOLD);
          }
          break;

        // waiting for more data in Rx FIFO buffer
        case READ_DATA:
          if (GPIO_READ(this->gdo0)) { // assert when Rx FIFO buffer threshold reached
            if ((rxLoop.bytesLeft < MAX_FIXED_LENGTH) && (rxLoop.cc1101Mode == INFINITE)) {
              cc1101.write_reg(CC1101_PKTCTRL0, FIXED_PACKET_LENGTH);
              rxLoop.cc1101Mode = FIXED;
            }
            // Do not empty the Rx FIFO (See the CC1101 SWRZ020E errata note)
            uint8_t bytesInFIFO = cc1101.read_status(CC1101_RXBYTES) & 0x7F;
            cc1101.read_burst(CC1101_RXFIFO, rxLoop.pByteIndex, bytesInFIFO - 1);

            rxLoop.bytesLeft  -= (bytesInFIFO - 1);
            rxLoop.pByteIndex += (bytesInFIFO - 1);
            rxLoop.bytesRx    += (bytesInFIFO - 1);
            max_wait_time_    += extra_time_;
          }
          break;
      }

      uint8_t overfl = cc1101.read_status(CC1101_RXBYTES) & 0x80;
      // end of packet in length mode
      if ((!overfl) && (!GPIO_READ(gdo2))  && (rxLoop.state > WAIT_FOR_DATA)) {
        cc1101.read_burst(CC1101_RXFIFO, rxLoop.pByteIndex, (uint8_t)rxLoop.bytesLeft);
        rxLoop.bytesRx += rxLoop.bytesLeft;
        data_in.length  = rxLoop.bytesRx;
        this->returnFrame.rssi  = cc1101.get_rssi();
        this->returnFrame.lqi   = cc1101.get_lqi();
        ESP_LOGV(TAG, "Have %d bytes from CC1101 Rx, RSSI: %d dBm LQI: %d", rxLoop.bytesRx, this->returnFrame.rssi, this->returnFrame.lqi);
        if (rxLoop.length != data_in.length) {
          ESP_LOGE(TAG, "Length problem: req(%d) != rx(%d)", rxLoop.length, data_in.length);
        }
        if (this->syncMode) {
          ESP_LOGV(TAG, "Synchronus mode enabled.");
        }
        if (mBusDecode(data_in, this->returnFrame)) {
          rxLoop.complete = true;
          this->returnFrame.mode  = data_in.mode;
          this->returnFrame.block = data_in.block;
        }
        rxLoop.state = INIT_RX;
        return rxLoop.complete;
      }
      start(false);
    } while ((this->syncMode) && (rxLoop.state > WAIT_FOR_SYNC));
    return rxLoop.complete;
  }

  WMbusFrame RxLoop::get_frame() {
    return this->returnFrame;
  }

  bool RxLoop::start(bool force) {
    // waiting to long for next part of data?
    bool reinit_needed = ((GET_MILLIS() - sync_time_) > max_wait_time_) ? true: false;
    if (!force) {
      if (!reinit_needed) {
        // already in RX?
        if (cc1101.read_status(CC1101_MARCSTATE) == MARCSTATE_RX) {
          return false;
        }
      }
    }
    // init RX here, each time we're idle
    rxLoop.state = INIT_RX;
    sync_time_ = GET_MILLIS();
    max_wait_time_ = extra_time_;

    cc1101.strobe(CC1101_SIDLE);
    while((cc1101.read_status(CC1101_MARCSTATE) != MARCSTATE_IDLE));
    cc1101.strobe(CC1101_SFTX);  //flush Tx FIFO
    cc1101.strobe(CC1101_SFRX);  //flush Rx FIFO

    // Initialize RX info variable
    rxLoop.lengthField = 0;              // Length Field in the wM-Bus packet
    rxLoop.length      = 0;              // Total length of bytes to receive packet
    rxLoop.bytesLeft   = 0;              // Bytes left to to be read from the Rx FIFO
    rxLoop.bytesRx     = 0;              // Bytes read from Rx FIFO
    rxLoop.pByteIndex  = data_in.data;   // Pointer to current position in the byte array
    rxLoop.complete    = false;          // Packet received
    rxLoop.cc1101Mode  = INFINITE;       // Infinite or fixed CC1101 packet mode

    this->returnFrame.frame.clear();
    this->returnFrame.rssi  = 0;
    this->returnFrame.lqi   = 0;
    this->returnFrame.mode  = 'X';
    this->returnFrame.block = 'X';

    std::fill( std::begin( data_in.data ), std::end( data_in.data ), 0 );
    data_in.length      = 0;
    data_in.lengthField = 0;
    data_in.mode        = 'X';
    data_in.block       = 'X';

    // Set Rx FIFO threshold to 4 bytes
    cc1101.write_reg(CC1101_FIFOTHR, RX_FIFO_START_THRESHOLD);
    // Set infinite length 
    cc1101.write_reg(CC1101_PKTCTRL0, INFINITE_PACKET_LENGTH);

    cc1101.strobe(CC1101_SRX);
    while((cc1101.read_status(CC1101_MARCSTATE) != MARCSTATE_RX));

    rxLoop.state = WAIT_FOR_SYNC;

    return true; // this will indicate we just have re-started Rx
  }

}
}