#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/uart/uart.h"
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif

namespace esphome {
namespace t740uno {

class T740UNO : public Component, public uart::UARTDevice {
#ifdef USE_BINARY_SENSOR
   SUB_BINARY_SENSOR(calling_alert)
#endif

public:

   void loop() override;

   void open();
};

}  // namespace t740uno
}  // namespace esphome
