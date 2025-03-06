#include "t740uno.h"
#include "esphome/core/log.h"


namespace esphome {
namespace t740uno {

static const char *const TAG = "t740uno";


void T740UNO::loop() {
    while (available()) {
        recvData_();
    }
}


void MegaDesk::recvData_() {
    const int numChars = 2;
    const int numFields = 4; // read/store all 4 fields for simplicity, use only the last 3.
    // static variables allows segmented/char-at-a-time decodes
    static uint16_t receivedBytes[numFields];
    static uint8_t ndx = 0;
    int r; // read char/digit
  
    // read 2 chars
    while ((ndx < numChars) && ((r = read()) != -1))
    {
      if ((ndx == 0) && (r != '>'))
      {
        // first char is not Tx, keep reading...
        continue;
      }
      receivedBytes[ndx] = r;
      ++ndx;
    }
    // read ascii digits
    while ((ndx >= numChars) && ((r = readdigits_()) != -1)) {
      receivedBytes[ndx] = r;
      this->digits_ = 0; // clear
      if (++ndx == numFields) {
        // thats all 4 fields. parse/process them now and break-out.
        parseData_(receivedBytes[1],
                   receivedBytes[2],
                   receivedBytes[3]);
        ndx = 0;
        return;
      }
    }
  }
  

void T740UNO::dump_config() {
    ESP_LOGCONFIG(TAG, "T740 UNO V2");
    LOG_PIN("  T740 RX Pin: ", this->rx_pin_);
    LOG_PIN("  T740 TX Pin: ", this->tx_pin_);
    ESP_LOGCONFIG(TAG, "  T740 Baudrate: %d baud", this->baud_rate_);
  }

}  // namespace t740uno
}  // namespace esphome