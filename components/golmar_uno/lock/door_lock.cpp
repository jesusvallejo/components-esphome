#include "door_lock.h"
#include "esphome/core/log.h"

namespace esphome {
namespace golmar_uno {

static const char *const TAG = "golmar_uno.lock";

void GolmarDoorLock::unlock() {
        ESP_LOGD(TAG, "Unlock requested via lock entity");
        if (this->parent_ != nullptr) {
            this->parent_->open();
            // schedule closing of lock entity (if component supports it)
            this->parent_->lock_door_entity(4000);
        }
}

}  // namespace golmar_uno
}  // namespace esphome