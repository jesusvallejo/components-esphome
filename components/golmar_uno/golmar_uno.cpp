#include "golmar_uno.h"
#include "esphome/core/log.h"

namespace esphome {
namespace golmar_uno {

static const char *const TAG = "golmar_uno";

void GolmarUnoComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Golmar UNO component...");

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

  // Initialize confirmation callbacks to empty lambdas
  this->on_confirm_concierge_ = []() {};
  this->on_confirm_intercom_ = []() {};
}

void GolmarUnoComponent::loop() {
  while (this->available()) {
    uint8_t byte = this->read();
    this->handle_incoming_call(byte);
    this->handle_concierge_confirm(byte);
  }
}

void GolmarUnoComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Golmar UNO:");
  ESP_LOGCONFIG(TAG, "  Intercom ID: 0x%02X", this->intercom_id_);
  ESP_LOGCONFIG(TAG, "  Concierge ID: 0x%02X", this->concierge_id_);
#ifdef USE_BINARY_SENSOR
  LOG_BINARY_SENSOR("  ", "Incoming Call", this->calling_alert_binary_sensor_);
#endif
#ifdef USE_BUTTON
  LOG_BUTTON("  ", "Unlock Door", this->unlock_door_button_);
#endif
#ifdef USE_SWITCH
  LOG_SWITCH("  ", "Unlock Door", this->unlock_door_switch_);
#endif
#ifdef USE_LOCK
  LOG_LOCK("  ", "Door Lock", this->door_lock_);
#endif
}

void GolmarUnoComponent::handle_incoming_call(uint8_t byte) {
  const std::array<uint8_t, PAYLOAD_SIZE> payload = {
      INTERCOM_ADDRESS1, INTERCOM_ADDRESS2, this->intercom_id_, INTERCOM_CALL_COMMAND};

  this->process_payload(byte, payload, this->incoming_match_index_, "Incoming call detected", [this]() {
#ifdef USE_BINARY_SENSOR
    if (this->calling_alert_binary_sensor_ != nullptr) {
      this->calling_alert_binary_sensor_->publish_state(true);
      this->set_timeout("call_alert_off", DEFAULT_CALL_ALERT_DURATION_MS, [this]() {
        if (this->calling_alert_binary_sensor_ != nullptr) {
          this->calling_alert_binary_sensor_->publish_state(false);
        }
      });
    }
#endif
  });
}

void GolmarUnoComponent::handle_concierge_confirm(uint8_t byte) {
  const std::array<uint8_t, PAYLOAD_SIZE> payload = {
      CONCIERGE_ADDRESS1, CONCIERGE_ADDRESS2, this->concierge_id_, CONFIRM_COMMAND};

  this->process_payload(byte, payload, this->concierge_confirm_match_index_,
                        "Concierge confirmation received", [this]() {
                          if (this->unlock_sequence_active_ && this->on_confirm_concierge_) {
                            this->on_confirm_concierge_();
                          }
                        });
}

void GolmarUnoComponent::handle_intercom_confirm(uint8_t byte) {
  const std::array<uint8_t, PAYLOAD_SIZE> payload = {
      INTERCOM_ADDRESS1, INTERCOM_ADDRESS2, this->intercom_id_, CONFIRM_COMMAND};

  this->process_payload(byte, payload, this->intercom_confirm_match_index_,
                        "Intercom confirmation received", [this]() {
                          if (this->on_confirm_intercom_) {
                            this->on_confirm_intercom_();
                          }
                        });
}

void GolmarUnoComponent::process_payload(uint8_t byte,
                                          const std::array<uint8_t, PAYLOAD_SIZE> &payload,
                                          size_t &match_index, const char *message,
                                          const std::function<void()> &on_match) {
  if (byte == payload[match_index]) {
    match_index++;
    if (match_index == payload.size()) {
      ESP_LOGD(TAG, "%s", message);
      if (on_match) {
        on_match();
      }
      match_index = 0;
    }
  } else {
    // Reset or start new match if byte matches first byte of payload
    match_index = (byte == payload[0]) ? 1 : 0;
  }
}

void GolmarUnoComponent::write_payload(uint8_t address1, uint8_t address2,
                                        uint8_t address3, uint8_t command) {
  ESP_LOGV(TAG, "Writing payload: 0x%02X 0x%02X 0x%02X 0x%02X", address1, address2, address3, command);
  const std::array<uint8_t, PAYLOAD_SIZE> payload = {address1, address2, address3, command};
  this->write_array(payload.data(), payload.size());
}

void GolmarUnoComponent::write_concierge_command(uint8_t command) {
  ESP_LOGD(TAG, "Sending concierge command: 0x%02X", command);
  this->write_payload(CONCIERGE_ADDRESS1, CONCIERGE_ADDRESS2, this->concierge_id_, command);
}

void GolmarUnoComponent::write_intercom_command(uint8_t command) {
  ESP_LOGD(TAG, "Sending intercom command: 0x%02X", command);
  this->write_payload(INTERCOM_ADDRESS1, INTERCOM_ADDRESS2, this->intercom_id_, command);
}

void GolmarUnoComponent::clear_bus() {
  ESP_LOGD(TAG, "Clearing bus");
  this->write_payload(NO_ADDRESS, NO_ADDRESS, NO_ADDRESS, CLEAR_BUS_COMMAND);
}

void GolmarUnoComponent::on_communication_error() {
  ESP_LOGE(TAG, "Communication error: No confirmation received");
#ifdef USE_LOCK
  if (this->door_lock_ != nullptr) {
    this->door_lock_->publish_state(lock::LockState::LOCK_STATE_JAMMED);
  }
#endif
}

void GolmarUnoComponent::unlock() {
  if (this->unlock_sequence_active_) {
    ESP_LOGW(TAG, "Unlock sequence already in progress, ignoring request");
    return;
  }

  ESP_LOGI(TAG, "Starting unlock sequence");

#ifdef USE_LOCK
  if (this->door_lock_ != nullptr) {
    this->door_lock_->publish_state(lock::LockState::LOCK_STATE_UNLOCKING);
  }
#endif

  this->clear_bus();
  this->unlock_sequence_active_ = true;

  // Step 1: After clearing bus, send call command to concierge
  this->set_timeout("unlock_step1", DEFAULT_INTER_COMMAND_DELAY_MS, [this]() {
    this->write_concierge_command(CONCIERGE_CALL_COMMAND);

    // Timeout if no confirmation received
    this->set_timeout("unlock_timeout", CONFIRMATION_TIMEOUT_MS, [this]() {
      if (this->unlock_sequence_active_) {
        this->on_communication_error();
        this->unlock_sequence_active_ = false;
      }
    });

    // Safety: always clear bus after delay
    this->set_timeout("unlock_clear_bus", CLEAR_BUS_DELAY_MS, [this]() {
      this->clear_bus();
      this->unlock_sequence_active_ = false;
    });

    // Step 2: On confirmation, send unlock command
    this->on_confirm_concierge_ = [this]() {
      this->cancel_timeout("unlock_timeout");
      this->set_timeout("unlock_step2", DEFAULT_INTER_COMMAND_DELAY_MS, [this]() {
        this->write_concierge_command(CONCIERGE_UNLOCK_COMMAND);

        // Step 3: On second confirmation, mark as unlocked
        this->on_confirm_concierge_ = [this]() {
          this->cancel_timeout("unlock_clear_bus");
          this->unlock_sequence_active_ = false;
          ESP_LOGI(TAG, "Door unlocked successfully");
#ifdef USE_LOCK
          if (this->door_lock_ != nullptr) {
            this->door_lock_->publish_state(lock::LockState::LOCK_STATE_UNLOCKED);
          }
#endif
          // Reset callback
          this->on_confirm_concierge_ = []() {};
        };
      });
    };
  });
}

#ifdef USE_SWITCH
void GolmarUnoComponent::schedule_switch_off(uint32_t delay_ms) {
  if (this->unlock_door_switch_ != nullptr) {
    this->set_timeout("switch_off", delay_ms, [this]() {
      if (this->unlock_door_switch_ != nullptr) {
        this->unlock_door_switch_->publish_state(false);
      }
    });
  }
}
#endif

#ifdef USE_LOCK
void GolmarUnoComponent::schedule_door_lock(uint32_t delay_ms) {
  if (this->door_lock_ != nullptr) {
    this->set_timeout("door_lock", delay_ms, [this]() {
      if (this->door_lock_ != nullptr) {
        this->door_lock_->publish_state(lock::LockState::LOCK_STATE_LOCKED);
      }
    });
  }
}
#endif

}  // namespace golmar_uno
}  // namespace esphome
