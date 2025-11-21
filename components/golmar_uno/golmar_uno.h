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

namespace esphome::golmar_uno {

class golmar_uno_component : public Component, public uart::UARTDevice {
#ifdef USE_BINARY_SENSOR
   SUB_BINARY_SENSOR(calling_alert)
#endif
#ifdef USE_BUTTON
  SUB_BUTTON(open_door)
#endif

protected:
   uint8_t intercom_id_{};
   uint8_t concierge_id_{};
   bool matter_hub_compatible_{};

public:

   void loop() override;
   void dump_config() override;
   void open();
   //void setup() override;
   void set_intercom_id(uint8_t intercom_id) { this->intercom_id_ = intercom_id; }
   void set_concierge_id(uint8_t concierge_id) { this->concierge_id_ = concierge_id; }
   void set_matter_hub_compatible(bool compatible) { this->matter_hub_compatible_ = compatible; }
   void set_incoming_call(incoming_call *incoming_call) { this->calling_alert_binary_sensor_ = incoming_call; }
   void set_open_door_button(open_door_button *open_door) { this->open_door_button_ = open_door; }

};

}  // namespace esphome::golmar_uno