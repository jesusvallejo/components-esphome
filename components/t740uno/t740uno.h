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

namespace esphome {
namespace t740uno {

class T740UNOComponent : public Component, public uart::UARTDevice {
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
};

}  // namespace t740uno
}  // namespace esphome
