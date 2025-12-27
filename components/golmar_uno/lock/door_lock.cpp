#include "door_lock.h"
#include "esphome/core/log.h"

namespace esphome {
namespace golmar_uno {

static const char *const TAG = "golmar_uno.lock";

void door_lock::unlock() {
        ESP_LOGD(TAG, "Unlock requested via lock entity");
        if (this->parent_ != nullptr) {
            this->parent_->open();
        }
}

void door_lock::lock() {
    // For this simple implementation, we do not support locking via the lock entity.
    ESP_LOGW(TAG, "Lock requested via lock entity, but locking is not supported. reseting to locked state.");
    if (this->parent_ != nullptr) {
            this->parent_->schedule_door_lock(4000);
        }
}

void door_lock::control(const lock::LockCall &call) {
    this->unlock();
    this->lock();
}

}  // namespace golmar_uno
}  // namespace esphome