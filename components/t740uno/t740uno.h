#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"


namespace esphome {
namespace t740uno {

class T740UNO : public Component, public uart::UARTDevice {
 public:
   void loop() override;
   void dump_config() override;
};


}  // namespace t740uno
}  // namespace esphome