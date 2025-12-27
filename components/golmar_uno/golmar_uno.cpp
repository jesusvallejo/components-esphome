#include "golmar_uno.h"
#include "esphome/core/log.h"
#include <array>

namespace esphome {
namespace golmar_uno {

static const char *TAG = "golmar_uno.component";

void golmar_uno_component::dump_config() {
#ifdef USE_BINARY_SENSOR
  LOG_BINARY_SENSOR(" ", "Incoming Call Binary Sensor", this->calling_alert_binary_sensor_);
#endif
#ifdef USE_BUTTON
  LOG_BUTTON(" ", "Open Door Button", this->open_door_button_);
#endif
#ifdef USE_SWITCH
  LOG_SWITCH(" ", "Open Door Switch", this->open_door_switch_);
#endif
#ifdef USE_LOCK
  LOG_LOCK(" ", "Unlock door", this->door_lock_);
#endif
}

void golmar_uno_component::setup() {
  #ifdef USE_BINARY_SENSOR
    if (this->calling_alert_binary_sensor_ != nullptr) {
      this->calling_alert_binary_sensor_->publish_state(false);
    }
  #endif
}


void golmar_uno_component::loop() {
  const uint8_t INTERCOM_ADDRESS1 = 0x00;
  const uint8_t INTERCOM_ADDRESS2 = 0x00;
  const uint8_t INTERCOM_ADDRESS3 = this->intercom_id_;
  const uint8_t INTERCOM_COMMAND = 0x37;

  const uint8_t target_payload[] = {INTERCOM_ADDRESS1, INTERCOM_ADDRESS2, INTERCOM_ADDRESS3, INTERCOM_COMMAND};
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
                this->set_timeout(2000, [this]() { this->calling_alert_binary_sensor_->publish_state(false); });
            }
        #endif
          match_index = 0;
        }
      } else {
        match_index = (byte == target_payload[0]) ? 1 : 0;
      }
    }
}


void golmar_uno_component::open() {
  ESP_LOGD(TAG, "Unlock door sequence started");
  const uint8_t CONCIERGE_ADDRESS1 = 0x00;
  const uint8_t CONCIERGE_ADDRESS2 = 0x00;
  const uint8_t CONCIERGE_ADDRESS3 = this->concierge_id_;

  const uint8_t CONCIERGE_CALL_COMMAND = 0x22;
  const uint8_t CONCIERGE_OPEN_COMMAND = 0x90;
  const uint8_t CLEAR_BUS_COMMAND = 0x11;

  const std::array<uint8_t, 4> call_payload = {CONCIERGE_ADDRESS1, CONCIERGE_ADDRESS2, CONCIERGE_ADDRESS3, CONCIERGE_CALL_COMMAND};
  const std::array<uint8_t, 4> open_payload = {CONCIERGE_ADDRESS1, CONCIERGE_ADDRESS2, CONCIERGE_ADDRESS3, CONCIERGE_OPEN_COMMAND};
  const std::array<uint8_t, 4> clear_bus_payload = {CONCIERGE_ADDRESS1, CONCIERGE_ADDRESS2, CONCIERGE_ADDRESS3, CLEAR_BUS_COMMAND};

  this->write_array(clear_bus_payload.data(), clear_bus_payload.size()); // clear
  ESP_LOGD(TAG, "Clear bus command sent");
  this->set_timeout(500, [this,call_payload,open_payload,clear_bus_payload]() {
    this->write_array(call_payload.data(), call_payload.size()); // call
    ESP_LOGD(TAG, "Concierge call command sent");
    this->set_timeout(500, [this,open_payload,clear_bus_payload]() {
      this->write_array(open_payload.data(), open_payload.size()); // open
      ESP_LOGD(TAG, "Unlock door command sent");
      this->set_timeout(4000, [this,clear_bus_payload]() {
        this->write_array(clear_bus_payload.data(), clear_bus_payload.size()); // clear
        ESP_LOGD(TAG, "Clear bus command sent");
        #ifdef USE_LOCK
          this->door_lock_->publish_state(true);
        #endif
      });
    });
  });
}

#ifdef USE_SWITCH
void golmar_uno_component::schedule_switch_off(uint32_t delay_ms) {
  if (this->open_door_switch_ != nullptr) {
    this->set_timeout(delay_ms, [this]() {
      this->open_door_switch_->publish_state(false);
    });
  }
}
#endif

#ifdef USE_LOCK
void golmar_uno_component::lock_door_lock(uint32_t delay_ms) {
  if (this->door_lock_ != nullptr) {
    this->set_timeout(delay_ms, [this]() {
      this->door_lock_->publish_state(false);
    });
  }
}
#endif


}  // namespace golmar_uno
}  // namespace esphome 