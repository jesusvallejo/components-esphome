#pragma once

#include "esphome/components/switch/switch.h"
#include "../golmar_uno.h"

namespace esphome {
namespace golmar_uno {

/**
 * @brief Switch entity to trigger door unlock sequence.
 *
 * When turned on, initiates the unlock sequence and automatically
 * turns off after a delay.
 */
class UnlockDoorSwitch : public switch_::Switch, public Parented<GolmarUnoComponent> {
 protected:
  void write_state(bool state) override;
};

}  // namespace golmar_uno
}  // namespace esphome
