#include "open_door_button.h"
#include "esphome/core/log.h"

namespace esphome {
namespace t740uno {

static const char *const TAG = "t740uno.button";

void OpenDoorButton::press_action() {
    ESP_LOGD(TAG, "Sending data..."); 
    this->parent_->open(); 
}

void OpenDoorButton::dump_config() { LOG_BUTTON("", "T740 UNO Button", this);}

}  // namespace t740uno
}  // namespace esphome