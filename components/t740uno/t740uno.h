#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace t740uno {

class T740Uno : public Component, public uart::UARTDevice {
 public:
  T740Uno(UARTComponent *parent) : UARTDevice(parent) {}

  void setup() override;
  void loop() override;
  void open();

  void set_incoming_call(binary_sensor::BinarySensor *incoming_call) { incoming_call_ = incoming_call; }

 protected:
  binary_sensor::BinarySensor *incoming_call_;
};

}  // namespace t740uno
}  // namespace esphome
