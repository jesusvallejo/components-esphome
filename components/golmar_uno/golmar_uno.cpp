#include "golmar_uno.h"
#include "esphome/core/log.h"
#include "esphome/components/lock/lock.h"
#include <array>

namespace esphome::golmar_uno {

static constexpr char TAG[] = "golmar_uno.component";

void golmar_uno_component::dump_config() {
#ifdef USE_BINARY_SENSOR
  LOG_BINARY_SENSOR(" ", "Incoming Call Binary Sensor", this->calling_alert_binary_sensor_);
#endif
#ifdef USE_BUTTON
  LOG_BUTTON(" ", "Unlock Door Button", this->unlock_door_button_);
#endif
#ifdef USE_SWITCH
  LOG_SWITCH(" ", "Unlock Door Switch", this->unlock_door_switch_);
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

  #ifdef USE_LOCK
    if (this->door_lock_ != nullptr) {
      this->door_lock_->publish_state(lock::LockState::LOCK_STATE_LOCKED);
    }
  #endif

}


void golmar_uno_component::loop() {
    while (available()) {
      uint8_t byte = read();
      this->process_incoming_byte_(byte);
    }
}

void golmar_uno_component::process_incoming_byte_(uint8_t byte) {
  const std::array<uint8_t, 4> target_payload = {INTERCOM_ADDRESS1, INTERCOM_ADDRESS2, this->intercom_id_, INTERCOM_COMMAND};

  if (byte == target_payload[this->match_index_]) {
    this->match_index_++;
    if (this->match_index_ == target_payload.size()) {
      ESP_LOGD(TAG, "Incoming call detected");
#ifdef USE_BINARY_SENSOR
      if (this->calling_alert_binary_sensor_ != nullptr) {
        this->calling_alert_binary_sensor_->publish_state(true);
        this->set_timeout(2000, [this]() { this->calling_alert_binary_sensor_->publish_state(false); });
      }
#endif
      this->match_index_ = 0;
    }
  } else {
    this->match_index_ = (byte == target_payload[0]) ? 1 : 0;
  }
}

void golmar_uno_component::clear_bus() {
  ESP_LOGD(TAG, "Clear bus command initiated");
  const std::array<uint8_t, 4> clear_bus_payload = {CONCIERGE_ADDRESS1, CONCIERGE_ADDRESS2, this->concierge_id_, CLEAR_BUS_COMMAND};

  this->write_array(clear_bus_payload.data(), clear_bus_payload.size()); // clear
  ESP_LOGD(TAG, "Clear bus command sent");
}


void golmar_uno_component::unlock() {
  ESP_LOGD(TAG, "Unlock door sequence started");
  const std::array<uint8_t, 4> call_payload = {CONCIERGE_ADDRESS1, CONCIERGE_ADDRESS2, this->concierge_id_, CONCIERGE_CALL_COMMAND};
  const std::array<uint8_t, 4> unlock_payload = {CONCIERGE_ADDRESS1, CONCIERGE_ADDRESS2, this->concierge_id_, CONCIERGE_UNLOCK_COMMAND};
  const std::array<uint8_t, 4> clear_bus_payload = {CONCIERGE_ADDRESS1, CONCIERGE_ADDRESS2, this->concierge_id_, CLEAR_BUS_COMMAND};

  #ifdef USE_LOCK
    if (this->door_lock_ != nullptr)
      this->door_lock_->publish_state(lock::LockState::LOCK_STATE_UNLOCKING);
  #endif

  this->write_array(clear_bus_payload.data(), clear_bus_payload.size()); // clear
  ESP_LOGD(TAG, "Clear bus command sent");
  this->set_timeout(500, [this,call_payload,unlock_payload,clear_bus_payload]() {
    this->write_array(call_payload.data(), call_payload.size()); // call
    ESP_LOGD(TAG, "Concierge call command sent");
    this->set_timeout(500, [this,unlock_payload,clear_bus_payload]() {
      this->write_array(unlock_payload.data(), unlock_payload.size()); // unlock
      ESP_LOGD(TAG, "Unlock door command sent");
      #ifdef USE_LOCK
          if (this->door_lock_ != nullptr)
            this->door_lock_->publish_state(lock::LockState::LOCK_STATE_UNLOCKED);
      #endif
      this->set_timeout(4000, [this,clear_bus_payload]() {
        this->write_array(clear_bus_payload.data(), clear_bus_payload.size()); // clear
        ESP_LOGD(TAG, "Clear bus command sent");

      });
    });
  });
}

#ifdef USE_SWITCH
void golmar_uno_component::schedule_switch_off(uint32_t delay_ms) {
  if (this->unlock_door_switch_ != nullptr) {
    this->set_timeout(delay_ms, [this]() {
      this->unlock_door_switch_->publish_state(false);
    });
  }
}
#endif

#ifdef USE_LOCK
void golmar_uno_component::schedule_door_lock(uint32_t delay_ms) {
  if (this->door_lock_ != nullptr) {
    this->set_timeout(delay_ms, [this]() {
      this->door_lock_->publish_state(lock::LockState::LOCK_STATE_LOCKED);
    });
  }
}
#endif


}  // namespace esphome::golmar_uno