#pragma once

#include "esphome/core/component.h"
#include "esphome/components/lock/lock.h"
#include "../golmar_uno.h"

namespace esphome {
namespace golmar_uno {

class door_lock : public lock::Lock, public Parented<golmar_uno_component> {
 public:
  door_lock() = default;
 protected:
  void unlock() override;
};

}  // namespace golmar_uno
}  // namespace esphome  