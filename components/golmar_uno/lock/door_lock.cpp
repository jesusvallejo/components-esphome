#include "door_lock.h"
#include "esphome/core/log.h"

namespace esphome {
namespace golmar_uno {

static const char *const TAG = "golmar_uno.lock";

void unlock_door_lock::press_action() {
    ESP_LOGD(TAG, "Sending pressed intercom button payload"); 
    this->parent_->unlock_door_lock(); 
}

}  // namespace golmar_uno
} 