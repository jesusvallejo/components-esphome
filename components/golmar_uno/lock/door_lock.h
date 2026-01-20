#pragma once

#include "esphome/core/component.h"
#include "esphome/components/lock/lock.h"
#include "../golmar_uno.h"

namespace esphome {
namespace golmar_uno {

/// Duration before auto-locking door (ms)
static constexpr uint32_t LOCK_AUTO_LOCK_DELAY_MS = 10000;

/**
 * @brief Lock entity for door control.
 *
 * Supports unlocking the door (which auto-locks after a delay).
 * The lock operation is a no-op as this is controlled by the intercom system.
 */
class DoorLock : public Component, public lock::Lock, public Parented<GolmarUnoComponent> {
 public:
  float get_setup_priority() const override { return setup_priority::DATA - 1.0f; }
  
  void setup() override {
    // Configure lock traits - supports open action for momentary unlock
    this->traits.set_supports_open(true);
  }
  
  void control(const lock::LockCall &call) override;
};

}  // namespace golmar_uno
}  // namespace esphome