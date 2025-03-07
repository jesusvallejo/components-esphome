#include "t740uno.h"
#include "esphome/core/log.h"

namespace esphome {
namespace t740uno {

static const char *const TAG = "t740uno";

void T740UNOComponent::dump_config() {
  #ifdef USE_BINARY_SENSOR
    LOG_BINARY_SENSOR(" ", "Incoming Call Binary Sensor", this->calling_alert_binary_sensor_);
  #endif
  #ifdef USE_BUTTON
    LOG_BUTTON(" ", "Open Door Button", this->open_door_button_);
  #endif

}

void T740UNOComponent::loop() {
  while (available()) {
    if (read() == 0x34) {
      ESP_LOGD(TAG, "Incoming call detected");
      #ifdef USE_BINARY_SENSOR
        if (this->calling_alert_binary_sensor_ != nullptr) {
            this->calling_alert_binary_sensor_->publish_state(true);
            this->set_timeout(1000, [this]() { this->calling_alert_binary_sensor_->publish_state(false); });
        }
      #endif
    }
  }
}

void T740UNOComponent::open() {
  ESP_LOGD(TAG, "Open command sent");
  this->write_byte(0x34);
}

}  // namespace t740uno
}  // namespace esphome
