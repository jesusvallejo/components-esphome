#pragma once

#include "esphome/core/component.h"
#include "esphome/components/button/button.h"
#include "../t740uno.h"

namespace esphome {
namespace t740uno {

class OpenDoorButton : public button::Button, public Parented<T740UNOComponent> {
 public:
  OpenDoorButton() = default;
  
  void dump_config() override;

 protected:
  void press_action() override;
};

}  // namespace t740uno
}  // namespace esphome