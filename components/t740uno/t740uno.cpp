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


void T740UNOComponent::loop() {
    static const uint8_t target_payload[] = {0x80, 0x80, 0xc9, 0xdb, 0x80, 0x80, 0xc9, 0xc1};
    static size_t match_index = 0;

    while (available()) {
      uint8_t byte = read();
      if (byte == target_payload[match_index]) {
        match_index++;
        if (match_index == sizeof(target_payload)) {
                ESP_LOGD(TAG, "Incoming call detected");
        #ifdef USE_BINARY_SENSOR
            if (this->calling_alert_binary_sensor_ != nullptr) {
                this->calling_alert_binary_sensor_->publish_state(true);
                this->set_timeout(1000, [this]() { this->calling_alert_binary_sensor_->publish_state(false); });
            }
        #endif
          match_index = 0;
          
        }
      } else {
        match_index = (byte == target_payload[0]) ? 1 : 0;
      }
    }
}


void T740UNOComponent::open() {
  ESP_LOGD(TAG, "Open command sent");
  this->write_byte(0x34);
}

}  // namespace t740uno
}  // namespace esphome
