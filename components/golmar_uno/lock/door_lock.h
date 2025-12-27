#pragma once

#include "esphome/core/component.h"
#include "esphome/components/lock/lock.h"
#include "../golmar_uno.h"

namespace esphome {
namespace golmar_uno {

class GolmarDoorLock : public lock::Lock, public Parented<golmar_uno_component> {
 public:
  GolmarDoorLock() = default;
 protected:
  void unlock();
};

}  // namespace golmar_uno
}  // namespace esphome  