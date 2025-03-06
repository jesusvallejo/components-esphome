#include "t740uno.h"
#include "esphome/core/log.h"

namespace esphome {
namespace t740uno {

static const char *const TAG = "t740uno";

void T740Uno::setup() {
  ESP_LOGCONFIG(TAG, "Setting up T740Uno...");
  // Add setup code here as needed
}

void T740Uno::loop() {
  while (available()) {
    if (read() == 0x34) {
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

void T740Uno::open() {
  ESP_LOGD(TAG, "Open command sent");
  this->write_byte(0x55);
}

}  // namespace t740uno
}  // namespace esphome
