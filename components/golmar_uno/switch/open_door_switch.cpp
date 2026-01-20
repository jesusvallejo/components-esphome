#include "open_door_switch.h"
#include "esphome/core/log.h"

namespace esphome {
namespace golmar_uno {

static const char *const TAG = "golmar_uno.switch";

void UnlockDoorSwitch::write_state(bool state) {
  if (state) {
    ESP_LOGD(TAG, "Unlock switch turned on");
    this->publish_state(true);
    if (this->parent_ != nullptr) {
      this->parent_->unlock();
    }
    // Schedule auto-off using Component's set_timeout
    this->set_timeout("auto_off", SWITCH_AUTO_OFF_DELAY_MS, [this]() {
      this->publish_state(false);
    });
  } else {
    // Cancel pending auto-off if manually turned off
    this->cancel_timeout("auto_off");
    this->publish_state(false);
  }
}

}  // namespace golmar_uno
}  // namespace esphome
