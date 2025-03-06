#pragma once

#include "esphome/components/button/button.h"
#include "../t740uno.h"

namespace esphome {
namespace t740uno {

class OpneDoorButton : public button::Button, public Parented<T740UNOComponent> {
 public:
  OpneDoorButton() = default;

 protected:
  void press_action() override;
};

}  // namespace t740uno
}  // namespace esphome