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
      // Cancel any pending auto-lock
      this->cancel_timeout("auto_lock");
      ESP_LOGD(TAG, "Lock requested - publishing locked state");
      this->publish_state(lock::LOCK_STATE_LOCKED);
      break;

    case lock::LOCK_STATE_UNLOCKED:
      ESP_LOGI(TAG, "Unlock requested via lock entity");
      if (this->parent_ != nullptr) {
        this->parent_->unlock();
      }
      // Schedule auto-lock using Component's set_timeout
      this->set_timeout("auto_lock", LOCK_AUTO_LOCK_DELAY_MS, [this]() {
        ESP_LOGD(TAG, "Auto-locking door");
        this->publish_state(lock::LOCK_STATE_LOCKED);
      });
      break;

    case lock::LOCK_STATE_NONE:
      // "Open" action - unlock without auto-lock (momentary)
      ESP_LOGI(TAG, "Open action requested via lock entity");
      if (this->parent_ != nullptr) {
        this->parent_->unlock();
      }
      break;

    default:
      ESP_LOGW(TAG, "Unsupported lock state requested");
      break;
  }
}

}  // namespace golmar_uno
}  // namespace esphome
