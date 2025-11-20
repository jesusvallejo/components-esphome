#include "open_door_button.h"
#include "esphome/core/log.h"

namespace esphome {
namespace golmar_uno {

static const char *const TAG = "golmar_uno.button";

void open_door_button::press_action() {
    ESP_LOGD(TAG, "Sending pressed intercom button payload"); 
    this->parent_->open(); 
}

}  // namespace golmar_uno
} 