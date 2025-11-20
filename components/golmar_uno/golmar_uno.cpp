#include "golmar_uno.h"
#include "esphome/core/log.h"

namespace esphome {
namespace golmar_uno {

static const char *TAG = "golmar_uno.component";

// void golmar_uno_component::setup() {
//   ESP_LOGD(TAG, "Setting up Golmar UNO component");
//}

void golmar_uno_component::dump_config() {
  #ifdef USE_BINARY_SENSOR
    LOG_BINARY_SENSOR(" ", "Incoming Call Binary Sensor", this->calling_alert_binary_sensor_);
  #endif
  #ifdef USE_BUTTON
    LOG_BUTTON(" ", "Open Door Button", this->open_door_button_);
  #endif

}


void golmar_uno_component::loop() {

    static const uint8_t INTERCOM_ADDRESS1 = 0x00;
    static const uint8_t INTERCOM_ADDRESS2 = 0x00;
    static const uint8_t INTERCOM_ADDRESS3 = this->intercom_id_;
    static const uint8_t INTERCOM_COMMAND = 0x37;

    static const uint8_t target_payload[] = {INTERCOM_HEADER1, INTERCOM_HEADER2, CONF_INTERCOM_ID, INTERCOM_COMMAND};
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
  ESP_LOGD(TAG, "Open command sent");
  static const uint8_t CONCIERGE_ADDRESS1 = 0x00;
  static const uint8_t CONCIERGE_ADDRESS2 = 0x00;
  static const uint8_t CONCIERGE_ADDRESS3 = this->concierge_id_;

  static const uint8_t CONCIERGE_CALL_COMMAND = 0x22;
  static const uint8_t CONCIERGE_OPEN_COMMAND = 0x90;
  static const uint8_t CLEAR_BUS_COMMAND = 0x11;
  
  static const uint8_t [] call_payload = {CONCIERGE_HEADER1, CONCIERGE_HEADER2, CONCIERGE_ADDRESS3, CONCIERGE_CALL_COMMAND};
  static const uint8_t [] open_payload = {CONCIERGE_HEADER1, CONCIERGE_HEADER2, CONCIERGE_ADDRESS3, CONCIERGE_OPEN_COMMAND};
  static const uint8_t [] clear_bus_payload = {CONCIERGE_HEADER1, CONCIERGE_HEADER2, CONCIERGE_ADDRESS3, CLEAR_BUS_COMMAND};

            uart.write_array(clear_bus_payload, 4); // clear
            this.set_timeout(500, [this]() {
                uart.write_array(call_payload, 4); // call
                this.set_timeout(500, [this]() {
                    uart.write_array(open_payload, 4); // open
                    this->set_timeout(4000, [this]() {
                        uart.write_array(clear_bus_payload, 4); // clear
                    });
                });
            });     
          }      

    }  // namespace golmar_uno
}  // namespace esphome



