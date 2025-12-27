#include "door_lock.h"
#include "esphome/core/log.h"

namespace esphome {
namespace golmar_uno {

static const char *const TAG = "golmar_uno.lock";

void door_lock::unlock() {
        ESP_LOGD(TAG, "Unlock requested via lock entity");
        if (this->parent_ != nullptr) {
            this->parent_->open();
            // schedule closing of lock entity (if component supports it)
            this->parent_->lock_door_lock(4000);
        }
}

void door_lock::control(const lock::LockCall &call) {
    // For this simple implementation, any control request will trigger an unlock.
    // We forward to the unlock() helper which triggers the intercom sequence
    // and schedules re-locking via the parent component.
    this->unlock();
}

}  // namespace golmar_uno
}  // namespace esphome