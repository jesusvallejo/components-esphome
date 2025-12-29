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

  #ifdef USE_BINARY_SENSOR // INITIAL STATE NO INCOMING CALL
    if (this->calling_alert_binary_sensor_ != nullptr) {
      this->calling_alert_binary_sensor_->publish_state(false);
    }
  #endif

  #ifdef USE_LOCK // INITIAL STATE DOOR LOCKED
    if (this->door_lock_ != nullptr) {
      this->door_lock_->publish_state(lock::LockState::LOCK_STATE_LOCKED);
    }
  #endif

}

void golmar_uno_component::loop() {
    while (available()) {
      uint8_t byte = read();
      this->detect_incoming_call_(byte);  // SHOULD IMPLEMENT OTHER MEANS TO DETECT OTHER PAYLOADS
    }
}

void golmar_uno_component::detect_incoming_call_(uint8_t byte) {
  const std::array<uint8_t, 4> target_payload = {INTERCOM_ADDRESS1, INTERCOM_ADDRESS2, this->intercom_id_, INTERCOM_CALL_COMMAND};

  if (byte == target_payload[this->match_index_]) {
    this->match_index_++;
    if (this->match_index_ == target_payload.size()) {

      #ifdef USE_BINARY_SENSOR
        ESP_LOGD(TAG, "Incoming call detected");
        if (this->calling_alert_binary_sensor_ != nullptr) {
          this->calling_alert_binary_sensor_->publish_state(true);
          this->set_timeout(DEFAULT_CALL_ALERT_DURATION_MS, [this]() { this->calling_alert_binary_sensor_->publish_state(false); });
        }
      #endif

      this->match_index_ = 0;
    }
  } else {
    this->match_index_ = (byte == target_payload[0]) ? 1 : 0;
  }
}

void golmar_uno_component::write_payload(uint8_t address1, uint8_t address2, uint8_t address3, uint8_t command) {
  ESP_LOGD(TAG, "Writing payload: 0x%02X 0x%02X 0x%02X 0x%02X", address1, address2, address3, command);
  const std::array<uint8_t, 4> payload = {address1, address2, address3, command};
  this->write_array(payload.data(), payload.size());
}

void golmar_uno_component::write_concierge_command(uint8_t command) {
  ESP_LOGD(TAG, "Writing concierge command: 0x%02X", command);
  this->write_payload(CONCIERGE_ADDRESS1, CONCIERGE_ADDRESS2, this->concierge_id_, command);
}

void golmar_uno_component::write_intercom_command(uint8_t command) {
  ESP_LOGD(TAG, "Writing intercom command: 0x%02X", command);
  this->write_payload(INTERCOM_ADDRESS1, INTERCOM_ADDRESS2, this->intercom_id_, command);
}

void golmar_uno_component::clear_bus() {
  this->write_payload(NO_ADDRESS, NO_ADDRESS, NO_ADDRESS, CLEAR_BUS_COMMAND);
  ESP_LOGD(TAG, "Clear bus command sent");
}

void golmar_uno_component::unlock() {
  ESP_LOGD(TAG, "Unlock door sequence started");
  #ifdef USE_LOCK
    if (this->door_lock_ != nullptr)
      this->door_lock_->publish_state(lock::LockState::LOCK_STATE_UNLOCKING);
  #endif

  clear_bus();
  this->set_timeout(DEFAULT_INTER_COMMAND_DELAY_DURATION_MS, [this]() {
    this->write_concierge_command(CONCIERGE_CALL_COMMAND);
    this->set_timeout(DEFAULT_INTER_COMMAND_DELAY_DURATION_MS, [this]() {
      this->write_concierge_command(CONCIERGE_UNLOCK_COMMAND);
      #ifdef USE_LOCK
          if (this->door_lock_ != nullptr)
            this->door_lock_->publish_state(lock::LockState::LOCK_STATE_UNLOCKED);
      #endif
      this->set_timeout(4000, [this]() {
        this->clear_bus();
      });
    });
  });
  ESP_LOGD(TAG, "Unlock door sequence commands scheduled");
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