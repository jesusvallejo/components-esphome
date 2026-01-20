#pragma once

#include "esphome/components/button/button.h"
#include "../golmar_uno.h"

namespace esphome {
namespace golmar_uno {

/**
 * @brief Button entity to trigger door unlock sequence.
 */
class UnlockDoorButton : public button::Button, public Parented<GolmarUnoComponent> {
 protected:
  void press_action() override;
};

}  // namespace golmar_uno
}  // namespace esphome