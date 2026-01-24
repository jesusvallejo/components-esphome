#include "rf_cc1101.h"

#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "freertos/task.h"

namespace esphome {
namespace wmbus {

namespace {

static const char *TAG = "rxLoop";

// GPIO helper functions
inline bool gpio_read(uint8_t pin) {
  return gpio_get_level(static_cast<gpio_num_t>(pin)) != 0;
}

inline void gpio_setup_input(uint8_t pin) {
  gpio_set_direction(static_cast<gpio_num_t>(pin), GPIO_MODE_INPUT);
}

inline void delay_ms(uint32_t ms) {
  vTaskDelay(pdMS_TO_TICKS(ms));
}

inline uint32_t get_millis() {
  return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

}  // namespace

uint8_t RxLoop::get_marc_state() {
  return cc1101.read_status(CC1101_MARCSTATE);
}

bool RxLoop::init(uint8_t mosi, uint8_t miso, uint8_t clk, uint8_t cs,
                  uint8_t gdo0, uint8_t gdo2, float freq, bool sync_mode) {
  sync_mode_ = sync_mode;
  gdo0_pin_ = gdo0;
  gdo2_pin_ = gdo2;

  gpio_setup_input(gdo0_pin_);
  gpio_setup_input(gdo2_pin_);

  // Initialize CC1101 SPI
  if (!cc1101.init_spi(mosi, miso, clk, cs)) {
    ESP_LOGE(TAG, "Failed to initialize SPI for CC1101");
    return false;
  }

  if (!cc1101.init()) {
    ESP_LOGE(TAG, "CC1101 initialization failed!");
    return false;
  }

  // Write T-Mode RF settings
  for (uint8_t i = 0; i < TMODE_RF_SETTINGS_LEN; ++i) {
    cc1101.write_reg(TMODE_RF_SETTINGS_BYTES[i << 1],
                     TMODE_RF_SETTINGS_BYTES[(i << 1) + 1]);
  }

  // Calculate and set frequency registers
  // Formula: FREQ = (f_carrier * 2^16) / f_xosc, where f_xosc = 26 MHz
  uint32_t freq_reg = static_cast<uint32_t>(freq * 65536.0f / 26.0f);
  uint8_t freq2 = (freq_reg >> 16) & 0xFF;
  uint8_t freq1 = (freq_reg >> 8) & 0xFF;
  uint8_t freq0 = freq_reg & 0xFF;

  ESP_LOGD(TAG, "Set CC1101 frequency to %.3f MHz [%02X %02X %02X]",
           freq, freq2, freq1, freq0);

  cc1101.write_reg(CC1101_FREQ2, freq2);
  cc1101.write_reg(CC1101_FREQ1, freq1);
  cc1101.write_reg(CC1101_FREQ0, freq0);

  // Calibrate frequency synthesizer
  cc1101.strobe(CC1101_SCAL);

  uint8_t version = cc1101.get_version();
  if (version == 0x00 || version == 0xFF) {
    ESP_LOGE(TAG, "CC1101 initialization FAILED!");
    return false;
  }

  ESP_LOGD(TAG, "CC1101 version: %d", version);
  cc1101.set_rx();
  ESP_LOGD(TAG, "CC1101 initialized");
  delay_ms(4);

  return true;
}

bool RxLoop::task() {
  do {
    switch (rx_loop_.state) {
      case RxState::INIT:
        start();
        return false;

      case RxState::WAIT_FOR_SYNC:
        // GDO2 asserts when SYNC word detected
        if (gpio_read(gdo2_pin_)) {
          rx_loop_.state = RxState::WAIT_FOR_DATA;
          sync_time_ = get_millis();
        }
        break;

      case RxState::WAIT_FOR_DATA:
        // GDO0 asserts when RX FIFO threshold reached
        if (gpio_read(gdo0_pin_)) {
          uint8_t preamble[2];
          
          // Read the first 3 bytes
          cc1101.read_burst(CC1101_RXFIFO, rx_loop_.buffer_ptr, 3);
          rx_loop_.bytes_received = 3;
          const uint8_t *current_byte = rx_loop_.buffer_ptr;

          // Check for Mode C
          if (*current_byte == WMBUS_MODE_C_PREAMBLE) {
            ++current_byte;
            rx_data_.mode = 'C';

            if (*current_byte == WMBUS_BLOCK_A_PREAMBLE) {
              // Block A
              ++current_byte;
              rx_loop_.length_field = *current_byte;
              rx_loop_.total_length = 2 + packetSize(rx_loop_.length_field);
              rx_data_.block = 'A';
            } else if (*current_byte == WMBUS_BLOCK_B_PREAMBLE) {
              // Block B
              ++current_byte;
              rx_loop_.length_field = *current_byte;
              rx_loop_.total_length = 2 + 1 + rx_loop_.length_field;
              rx_data_.block = 'B';
            } else {
              // Unknown type, reinit
              rx_loop_.state = RxState::INIT;
              return false;
            }
            // Don't include C "preamble"
            *rx_loop_.buffer_ptr = rx_loop_.length_field;
            rx_loop_.buffer_ptr += 1;

          } else if (decode3OutOf6(rx_loop_.buffer_ptr, preamble)) {
            // Mode T Block A
            rx_loop_.length_field = preamble[0];
            rx_data_.lengthField = rx_loop_.length_field;
            rx_loop_.total_length = byteSize(packetSize(rx_loop_.length_field));
            rx_data_.mode = 'T';
            rx_data_.block = 'A';
            rx_loop_.buffer_ptr += 3;
          } else {
            // Unknown mode, reinit
            rx_loop_.state = RxState::INIT;
            return false;
          }

          rx_loop_.bytes_remaining = rx_loop_.total_length - 3;

          if (rx_loop_.total_length < MAX_FIXED_LENGTH) {
            // Set CC1101 into fixed length mode
            cc1101.write_reg(CC1101_PKTLEN, static_cast<uint8_t>(rx_loop_.total_length));
            cc1101.write_reg(CC1101_PKTCTRL0, FIXED_PACKET_LENGTH);
            rx_loop_.length_mode = PacketLengthMode::FIXED;
          } else {
            // Set CC1101 into infinite mode
            cc1101.write_reg(CC1101_PKTLEN, static_cast<uint8_t>(rx_loop_.total_length % MAX_FIXED_LENGTH));
          }

          rx_loop_.state = RxState::READ_DATA;
          max_wait_time_ += EXTRA_TIME_MS;
          cc1101.write_reg(CC1101_FIFOTHR, RX_FIFO_THRESHOLD);
        }
        break;

      case RxState::READ_DATA:
        // GDO0 asserts when RX FIFO threshold reached
        if (gpio_read(gdo0_pin_)) {
          if (rx_loop_.bytes_remaining < MAX_FIXED_LENGTH && 
              rx_loop_.length_mode == PacketLengthMode::INFINITE) {
            cc1101.write_reg(CC1101_PKTCTRL0, FIXED_PACKET_LENGTH);
            rx_loop_.length_mode = PacketLengthMode::FIXED;
          }

          // Do not empty the RX FIFO completely (see CC1101 errata note)
          uint8_t bytes_in_fifo = cc1101.read_status(CC1101_RXBYTES) & 0x7F;
          uint8_t bytes_to_read = bytes_in_fifo - 1;
          
          cc1101.read_burst(CC1101_RXFIFO, rx_loop_.buffer_ptr, bytes_to_read);

          rx_loop_.bytes_remaining -= bytes_to_read;
          rx_loop_.buffer_ptr += bytes_to_read;
          rx_loop_.bytes_received += bytes_to_read;
          max_wait_time_ += EXTRA_TIME_MS;
        }
        break;
    }

    // Check for overflow or end of packet
    uint8_t rx_bytes = cc1101.read_status(CC1101_RXBYTES);
    bool overflow = (rx_bytes & 0x80) != 0;
    bool sync_deasserted = !gpio_read(gdo2_pin_);

    if (!overflow && sync_deasserted && rx_loop_.state > RxState::WAIT_FOR_DATA) {
      // Read remaining bytes
      cc1101.read_burst(CC1101_RXFIFO, rx_loop_.buffer_ptr, 
                        static_cast<uint8_t>(rx_loop_.bytes_remaining));
      rx_loop_.bytes_received += rx_loop_.bytes_remaining;
      rx_data_.length = rx_loop_.bytes_received;

      result_frame_.rssi = cc1101.get_rssi();
      result_frame_.lqi = cc1101.get_lqi();

      ESP_LOGV(TAG, "Received %d bytes, RSSI: %d dBm, LQI: %d",
               rx_loop_.bytes_received, result_frame_.rssi, result_frame_.lqi);

      if (rx_loop_.total_length != rx_data_.length) {
        ESP_LOGE(TAG, "Length mismatch: expected %d, received %d",
                 rx_loop_.total_length, rx_data_.length);
      }

      if (sync_mode_) {
        ESP_LOGV(TAG, "Synchronous mode enabled");
      }

      if (mBusDecode(rx_data_, result_frame_)) {
        rx_loop_.complete = true;
        result_frame_.mode = rx_data_.mode;
        result_frame_.block = rx_data_.block;
      }

      rx_loop_.state = RxState::INIT;
      return rx_loop_.complete;
    }

    start(false);

  } while (sync_mode_ && rx_loop_.state > RxState::WAIT_FOR_SYNC);

  return rx_loop_.complete;
}

WMbusFrame RxLoop::get_frame() {
  return result_frame_;
}

bool RxLoop::start(bool force) {
  // Check if reinit is needed due to timeout
  bool timeout = (get_millis() - sync_time_) > max_wait_time_;

  if (!force) {
    if (!timeout) {
      // Check if already in RX mode
      if (get_marc_state() == static_cast<uint8_t>(MarcState::RX)) {
        return false;
      }
    }
  }

  // Initialize RX
  rx_loop_.state = RxState::INIT;
  sync_time_ = get_millis();
  max_wait_time_ = EXTRA_TIME_MS;

  // Go to IDLE and flush FIFOs
  cc1101.strobe(CC1101_SIDLE);
  while (get_marc_state() != static_cast<uint8_t>(MarcState::IDLE)) {
    // Wait for IDLE state
  }
  cc1101.strobe(CC1101_SFTX);  // Flush TX FIFO
  cc1101.strobe(CC1101_SFRX);  // Flush RX FIFO

  // Reset RX loop state
  rx_loop_.length_field = 0;
  rx_loop_.total_length = 0;
  rx_loop_.bytes_remaining = 0;
  rx_loop_.bytes_received = 0;
  rx_loop_.buffer_ptr = rx_data_.data;
  rx_loop_.complete = false;
  rx_loop_.length_mode = PacketLengthMode::INFINITE;

  // Reset result frame
  result_frame_.frame.clear();
  result_frame_.rssi = 0;
  result_frame_.lqi = 0;
  result_frame_.mode = 'X';
  result_frame_.block = 'X';

  // Reset RX data buffer
  std::fill(std::begin(rx_data_.data), std::end(rx_data_.data), 0);
  rx_data_.length = 0;
  rx_data_.lengthField = 0;
  rx_data_.mode = 'X';
  rx_data_.block = 'X';

  // Configure for infinite packet mode with initial FIFO threshold
  cc1101.write_reg(CC1101_FIFOTHR, RX_FIFO_START_THRESHOLD);
  cc1101.write_reg(CC1101_PKTCTRL0, INFINITE_PACKET_LENGTH);

  // Enter RX mode
  cc1101.strobe(CC1101_SRX);
  while (get_marc_state() != static_cast<uint8_t>(MarcState::RX)) {
    // Wait for RX state
  }

  rx_loop_.state = RxState::WAIT_FOR_SYNC;
  return true;
}

}  // namespace wmbus
}  // namespace esphome