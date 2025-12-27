#include "door_lock.h"
#include "esphome/core/log.h"

namespace esphome {
namespace golmar_uno {

static const char *const TAG = "golmar_uno.lock";

void door_lock::unlock() {
    ESP_LOGD(TAG, "Unlock requested via lock entity");
    this->parent_->open();
}

}  // namespace golmar_uno
}  // namespace esphome