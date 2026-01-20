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
  ESP_LOGCONFIG(TAG, "  Call Alert Duration: %ums", this->call_alert_duration_ms_);
  ESP_LOGCONFIG(TAG, "  Unlock Timeout: %ums", this->unlock_timeout_ms_);
  ESP_LOGCONFIG(TAG, "  Command Delay: %ums", this->command_delay_ms_);
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
      this->set_timeout("call_alert_off", this->call_alert_duration_ms_, [this]() {
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
                          this->on_concierge_confirm();
                        });
}

void GolmarUnoComponent::on_concierge_confirm() {
  switch (this->unlock_state_) {
    case UnlockState::WAITING_CALL_CONFIRM:
      // First confirmation received - send unlock command
      this->cancel_timeout("unlock_call_timeout");
      this->unlock_state_ = UnlockState::WAITING_UNLOCK_CONFIRM;
      
      this->set_timeout("send_unlock_cmd", this->command_delay_ms_, [this]() {
        this->write_concierge_command(CONCIERGE_UNLOCK_COMMAND);
        
        // Set timeout for unlock confirmation
        this->set_timeout("unlock_confirm_timeout", this->unlock_timeout_ms_, [this]() {
          if (this->unlock_state_ == UnlockState::WAITING_UNLOCK_CONFIRM) {
            this->finish_unlock_sequence(false);
          }
        });
      });
      break;

    case UnlockState::WAITING_UNLOCK_CONFIRM:
      // Second confirmation received - door unlocked!
      this->cancel_timeout("unlock_confirm_timeout");
      this->finish_unlock_sequence(true);
      break;

    case UnlockState::IDLE:
      // Unexpected confirmation - ignore
      ESP_LOGD(TAG, "Received confirmation while idle, ignoring");
      break;
  }
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

void GolmarUnoComponent::clear_bus() {
  ESP_LOGD(TAG, "Clearing bus");
  this->write_payload(NO_ADDRESS, NO_ADDRESS, NO_ADDRESS, CLEAR_BUS_COMMAND);
}

void GolmarUnoComponent::finish_unlock_sequence(bool success) {
  this->unlock_state_ = UnlockState::IDLE;
  
  // Always clear the bus after unlock sequence completes
  this->set_timeout("cleanup_bus", this->command_delay_ms_, [this]() {
    this->clear_bus();
  });

  if (success) {
    ESP_LOGI(TAG, "Door unlocked successfully");
#ifdef USE_LOCK
    if (this->door_lock_ != nullptr) {
      this->door_lock_->publish_state(lock::LockState::LOCK_STATE_UNLOCKED);
    }
#endif
  } else {
    this->on_communication_error();
  }
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
  if (this->unlock_state_ != UnlockState::IDLE) {
    ESP_LOGW(TAG, "Unlock sequence already in progress, ignoring request");
    return;
  }

  ESP_LOGI(TAG, "Starting unlock sequence");

#ifdef USE_LOCK
  if (this->door_lock_ != nullptr) {
    this->door_lock_->publish_state(lock::LockState::LOCK_STATE_UNLOCKING);
  }
#endif

  // Step 1: Clear the bus first
  this->clear_bus();
  this->unlock_state_ = UnlockState::WAITING_CALL_CONFIRM;

  // Step 2: After clearing bus, send call command to concierge
  this->set_timeout("send_call_cmd", this->command_delay_ms_, [this]() {
    this->write_concierge_command(CONCIERGE_CALL_COMMAND);

    // Set timeout for call confirmation
    this->set_timeout("unlock_call_timeout", this->unlock_timeout_ms_, [this]() {
      if (this->unlock_state_ == UnlockState::WAITING_CALL_CONFIRM) {
        this->finish_unlock_sequence(false);
      }
    });
  });
}

}  // namespace golmar_uno
}  // namespace esphome
