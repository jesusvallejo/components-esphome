#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/uart/uart.h"
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif

namespace esphome::golmar_uno {

class golmar_uno_component : public Component, public uart::UARTDevice {
#ifdef USE_BINARY_SENSOR
   SUB_BINARY_SENSOR(calling_alert)
#endif
#ifdef USE_BUTTON
  SUB_BUTTON(open_door)
#endif
#ifdef USE_SWITCH
  SUB_SWITCH(open_door)
#endif

protected:
   uint8_t intercom_id_{};
   uint8_t concierge_id_{};

public:

   void loop() override;
   void setup() override;
   void dump_config() override;
   void open();
   void schedule_switch_off(uint32_t delay_ms);
   //void setup() override;
   void set_intercom_id(uint8_t intercom_id) { this->intercom_id_ = intercom_id; }
   void set_concierge_id(uint8_t concierge_id) { this->concierge_id_ = concierge_id; }
#ifdef USE_BINARY_SENSOR
   void set_calling_alert_binary_sensor_(binary_sensor::BinarySensor *calling_alert_binary_sensor) {
     this->calling_alert_binary_sensor_ = calling_alert_binary_sensor;
   }
#endif
#ifdef USE_BUTTON
   void set_open_door_button_(button::Button *open_door_button) {
     this->open_door_button_ = open_door_button;
   }
#endif
#ifdef USE_SWITCH
   void set_open_door_switch_(switch_::Switch *open_door_switch) {
     this->open_door_switch_ = open_door_switch;
   }
#endif

};

}  // namespace esphome::golmar_uno