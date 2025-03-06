#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace t740uno {

class T740UNO : public Component, public uart::UARTDevice {
 public:
   float get_setup_priority() const override { return setup_priority::DATA; }
   void loop() override;
   void dump_config() override;
   
   void set_ring_sensor(sensor::Sensor *sens) { this->ring_ = sens; }

 protected:
    sensor::Sensor *ring_{nullptr};
    void recvData_();
};



}  // namespace t740uno
}  // namespace esphome