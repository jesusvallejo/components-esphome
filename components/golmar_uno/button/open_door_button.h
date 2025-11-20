#pragma once

#include "esphome/core/component.h"
#include "esphome/components/button/button.h"
#include "../golmar_uno.h"

namespace esphome {
namespace golmar_uno {

class open_door_button : public button::Button, public Parented<golmar_uno_component> {
 public:
  open_door_button() = default;
 protected:
  void press_action() override;
};

}  // namespace golmar_uno
}  // namespace esphome