#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/preferences.h"
#include "esphome/components/uart/uart.h"
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif
#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif

namespace esphome::golmar_uno {

class golmar_uno_component;

class GolmarUnoNumber : public number::Number, public Component {
 public:
  void setup() override;
  void control(float value) override;
  void set_parent(golmar_uno_component *parent) { this->parent_ = parent; }

 protected:
  golmar_uno_component *parent_;
};

class golmar_uno_component : public Component, public uart::UARTDevice {
#ifdef USE_BINARY_SENSOR
   SUB_BINARY_SENSOR(calling_alert)
#endif
#ifdef USE_BUTTON
  SUB_BUTTON(open_door)
  SUB_BUTTON(detect_intercom_id)
#endif
#ifdef USE_NUMBER
  SUB_NUMBER(intercom_id)
#endif
#ifdef USE_SWITCH
  SUB_SWITCH(open_door)
#endif

protected:
   uint8_t intercom_id_{};
   uint8_t concierge_id_{};
   bool detection_mode_{false};
   size_t match_index_{0};
   uint8_t detected_id_buffer_{0};

 private:
  void handle_detection_mode(uint8_t byte);
  void handle_filtering_mode(uint8_t byte);

public:

   void loop() override;
   void setup() override;
   void dump_config() override;
   void open();
   void schedule_switch_off(uint32_t delay_ms);
   void set_intercom_id(uint8_t intercom_id);
   uint8_t get_intercom_id() { return this->intercom_id_; }
   void set_concierge_id(uint8_t concierge_id) { this->concierge_id_ = concierge_id; }
#ifdef USE_BINARY_SENSOR
   void set_calling_alert_binary_sensor_(binary_sensor::BinarySensor *calling_alert_binary_sensor) {
     this->calling_alert_binary_sensor_ = calling_alert_binary_sensor;
   }
#endif
#ifdef USE_BUTTON
   void set_open_door_button_(button::Button *open_door_button) { this->open_door_button_ = open_door_button; }
#endif
#ifdef USE_SWITCH
   void set_open_door_switch_(switch_::Switch *open_door_switch) {
     this->open_door_switch_ = open_door_switch;
   }
#endif

};

}  // namespace esphome::golmar_uno
