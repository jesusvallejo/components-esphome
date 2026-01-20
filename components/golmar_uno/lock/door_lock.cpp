#include "door_lock.h"
#include "esphome/core/log.h"

namespace esphome {
namespace golmar_uno {

static const char *const TAG = "golmar_uno.lock";

void DoorLock::control(const lock::LockCall &call) {
  auto state = call.get_state();
  if (!state.has_value()) {
    return;
  }

  switch (*state) {
    case lock::LOCK_STATE_LOCKED:
      // Locking is handled automatically by the intercom system
      ESP_LOGD(TAG, "Lock requested - door will auto-lock");
      this->publish_state(lock::LOCK_STATE_LOCKED);
      break;

    case lock::LOCK_STATE_UNLOCKED:
      ESP_LOGD(TAG, "Unlock requested via lock entity");
      if (this->parent_ != nullptr) {
        this->parent_->unlock();
        // Schedule auto-lock after delay
        this->parent_->schedule_door_lock(AUTO_LOCK_DELAY_MS);
      }
      break;

    default:
      break;
  }
}

}  // namespace golmar_uno
}  // namespace esphome