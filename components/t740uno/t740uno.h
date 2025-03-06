#pragma once

#include "esphome/components/component.h"
#include "esphome/components/uart/uart_device.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/core/controller.h"
#include "esphome/core/time.h"
#include "esphome/components/logger/logger.h"
#include <deque>

namespace esphome {
namespace t740uno {

class T740UNOComponent : public Component,
                                     public uart::UARTDevice,
                                     public Controller {
 public:
    T740UNOComponent(uart::UARTComponent *parent) : UARTDevice(parent) {}

    void set_logger(logger::Logger *logger) { this->logger_ = logger; }

    void setup() override;
    void loop() override;

    binary_sensor::BinarySensor *ring_sensor_{nullptr};
    void set_ring_sensor(binary_sensor::BinarySensor *sensor) { this->ring_sensor_ = sensor; }

    time_t ring_sensor_last_active_time_ = 0; 
    bool ring_active_ = false; 


    logger::Logger *logger_{nullptr};
    uart::UARTComponent *uart_{nullptr};


    esphome::action::Action<> *open_action();

 protected:
    std::deque<uint8_t> rx_buffer_;
};


class T740UNORingBinarySensor : public binary_sensor::BinarySensor {
 public:
  void set_parent(T740UNOComponent *parent) { this->parent_ = parent; }

 protected:
  T740UNOComponent *parent_{nullptr};
};


}  // namespace t740uno
}  // namespace esphome