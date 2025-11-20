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
public:

   void loop() override;
   void dump_config() override;
   void open();
   //void setup() override;
   void set_intercom_id(uint8_t id) { this->intercom_id_ = intercom_id; }
};

}  // namespace esphome::golmar_uno