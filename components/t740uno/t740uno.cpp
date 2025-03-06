#include "t740uno.h"
#include "esphome/core/log.h"

namespace esphome {
namespace t740uno {

static const char *const TAG = "t740uno";

// void T740UNOComponent::setup() {

// }


void T740UNOComponent::loop() {
  while (available()) {
    if (read() == 0x00) {
      ESP_LOGD(TAG, "Incoming call detected");
      #ifdef USE_BINARY_SENSOR
        if (this->calling_alert_binary_sensor_ != nullptr) {
            this->calling_alert_binary_sensor_->publish_state(true);
            this->set_timeout(90, [this]() { this->calling_alert_binary_sensor_->publish_state(false); });
        }
      #endif
    }
  }
}

void T740UNOComponent::open() {
  ESP_LOGD(TAG, "Open command sent");
  this->write_byte(0x55);
}

}  // namespace t740uno
}  // namespace esphome
