#include "open_door_button.h"
#include "esphome/core/log.h"

namespace esphome {
namespace t740uno {

static const char *const TAG = "t740uno.button";

void OpenDoorButton::press_action() {
    ESP_LOGD(TAG, "Sending pressed intercom button payload"); 
    this->parent_->open(); 
}

}  // namespace t740uno
}  // namespace