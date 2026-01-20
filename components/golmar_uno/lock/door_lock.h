#pragma once

#include "esphome/components/lock/lock.h"
#include "../golmar_uno.h"

namespace esphome {
namespace golmar_uno {

/**
 * @brief Lock entity for door control.
 *
 * Supports unlocking the door (which auto-locks after a delay).
 * The lock operation is a no-op as this is controlled by the intercom system.
 */
class DoorLock : public lock::Lock, public Parented<GolmarUnoComponent> {
 public:
  void control(const lock::LockCall &call) override;
};

}  // namespace golmar_uno
}  // namespace esphome