#include "t740uno.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"


namespace esphome {
namespace t740uno {

static const char *const TAG = "t740uno";


void T740UNO::loop() {
    while (available()) {
        // recvData_();
    }
}


// void T740UNO::recvData_() {
//     this->ring_->publish_state(1);
//   }
  

void T740UNO::dump_config() {
    ESP_LOGCONFIG(TAG, "T740 UNO V2");
    LOG_PIN("  T740 RX Pin: ", this->rx_pin_);
    LOG_PIN("  T740 TX Pin: ", this->tx_pin_);
    ESP_LOGCONFIG(TAG, "  T740 Baudrate: %d baud", this->baud_rate_);
  }

}  // namespace t740uno
}  // namespace esphome