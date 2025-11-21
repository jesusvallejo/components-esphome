#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "../golmar_uno.h"

namespace esphome {
namespace golmar_uno {

class open_door_switch : public switch_::Switch, public Parented<golmar_uno_component> {
 public:
  open_door_switch() = default;
 protected:
  void write_state(bool state) override;
};

}  // namespace golmar_uno
}  // namespace esphome
