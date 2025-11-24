#include "golmar_uno.h"
#include "esphome/core/log.h"
#include <array>

namespace esphome {
namespace golmar_uno {

void GolmarUnoNumber::setup() {
  this->publish_state(this->parent_->get_intercom_id());
}

void GolmarUnoNumber::control(float value) {
  this->parent_->set_intercom_id(value);
  this->publish_state(value);
}

void golmar_uno_component::set_intercom_id(uint8_t intercom_id) {
  this->intercom_id_ = intercom_id;
  global_preferences->set_preference((std::string("golmar_uno.") + this->get_object_id()).c_str(), intercom_id);
}

static const char *TAG = "golmar_uno.component";

void golmar_uno_component::dump_config() {
  LOG_BINARY_SENSOR(" ", "Incoming Call Binary Sensor", this->calling_alert_binary_sensor_);
#ifdef USE_BUTTON
  LOG_BUTTON(" ", "Open Door Button", this->open_door_button_);
  LOG_BUTTON(" ", "Detect Intercom ID Button", this->detect_intercom_id_button_);
#endif
#ifdef USE_NUMBER
  LOG_NUMBER(" ", "Intercom ID Number", this->intercom_id_number_);
#endif
#ifdef USE_SWITCH
  LOG_SWITCH(" ", "Open Door Switch", this->open_door_switch_);
#endif
}

void golmar_uno_component::setup() {
  #ifdef USE_BINARY_SENSOR
    if (this->calling_alert_binary_sensor_ != nullptr) {
      this->calling_alert_binary_sensor_->publish_state(false);
    }
  #endif
  #ifdef USE_BUTTON
    if (this->detect_intercom_id_button_ != nullptr) {
      this->detect_intercom_id_button_->add_on_press_callback([this]() {
        this->detection_mode_ = true;
        ESP_LOGD(TAG, "Intercom ID detection mode enabled");
        this->set_timeout(30000, [this]() {
          if (this->detection_mode_) {
            this->detection_mode_ = false;
            this->match_index_ = 0;
            ESP_LOGD(TAG, "Intercom ID detection mode timed out");
          }
        });
      });
    }
  #endif
  this->intercom_id_ = global_preferences->get_preference((std::string("golmar_uno.") + this->get_object_id()).c_str(), this->intercom_id_);
}


void golmar_uno_component::loop() {
  while (available()) {
    uint8_t byte = read();
    if (this->detection_mode_) {
      this->handle_detection_mode(byte);
    } else {
      this->handle_filtering_mode(byte);
    }
  }
}

void golmar_uno_component::handle_detection_mode(uint8_t byte) {
  const uint8_t INTERCOM_ADDRESS1 = 0x00;
  const uint8_t INTERCOM_ADDRESS2 = 0x00;
  const uint8_t INTERCOM_COMMAND = 0x37;

  if (this->match_index_ < 2) { // Check first two bytes only
    if (byte == INTERCOM_ADDRESS1 && this->match_index_ == 0) {
      this->match_index_++;
    } else if (byte == INTERCOM_ADDRESS2 && this->match_index_ == 1) {
      this->match_index_++;
    } else {
      this->match_index_ = 0;
    }
  } else if (this->match_index_ == 2) {
    this->detected_id_buffer_ = byte;
    this->match_index_++;
  } else if (this->match_index_ == 3) {
    if (byte == INTERCOM_COMMAND) {
      ESP_LOGD(TAG, "Detected intercom ID: 0x%02X", this->detected_id_buffer_);
      this->set_intercom_id(this->detected_id_buffer_);
      #ifdef USE_NUMBER
        if (this->intercom_id_number_ != nullptr) {
          this->intercom_id_number_->publish_state(this->detected_id_buffer_);
        }
      #endif
      this->detection_mode_ = false;
    }
    this->match_index_ = 0;
  }
}

void golmar_uno_component::handle_filtering_mode(uint8_t byte) {
  const uint8_t INTERCOM_ADDRESS1 = 0x00;
  const uint8_t INTERCOM_ADDRESS2 = 0x00;
  const uint8_t INTERCOM_COMMAND = 0x37;
  const uint8_t INTERCOM_ADDRESS3 = this->intercom_id_;
  const uint8_t target_payload[] = {INTERCOM_ADDRESS1, INTERCOM_ADDRESS2, INTERCOM_ADDRESS3, INTERCOM_COMMAND};

  if (byte == target_payload[this->match_index_]) {
    this->match_index_++;
    if (this->match_index_ == sizeof(target_payload)) {
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


void golmar_uno_component::open() {
  ESP_LOGD(TAG, "Open door sequence started");
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
      ESP_LOGD(TAG, "Open door command sent");
      this->set_timeout(4000, [this,clear_bus_payload]() {
        this->write_array(clear_bus_payload.data(), clear_bus_payload.size()); // clear
        ESP_LOGD(TAG, "Clear bus command sent");
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
}  // namespace golmar_uno
}  // namespace esphome 