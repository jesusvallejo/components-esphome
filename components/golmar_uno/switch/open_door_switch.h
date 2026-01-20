#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "../golmar_uno.h"

namespace esphome {
namespace golmar_uno {

/// Duration before auto-turning off switch (ms)
static constexpr uint32_t SWITCH_AUTO_OFF_DELAY_MS = 2000;

/**
 * @brief Switch entity to trigger door unlock sequence.
 *
 * When turned on, initiates the unlock sequence and automatically
 * turns off after a delay.
 */
class UnlockDoorSwitch : public Component, public switch_::Switch, public Parented<GolmarUnoComponent> {
 public:
  float get_setup_priority() const override { return setup_priority::DATA - 1.0f; }

 protected:
  void write_state(bool state) override;
};

}  // namespace golmar_uno
}  // namespace esphome
