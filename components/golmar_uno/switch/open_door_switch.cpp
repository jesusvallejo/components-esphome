#include "open_door_switch.h"
#include "esphome/core/log.h"

namespace esphome {
namespace golmar_uno {

static const char *const TAG = "golmar_uno.switch";

void UnlockDoorSwitch::write_state(bool state) {
  if (state) {
    ESP_LOGD(TAG, "Unlock switch turned on");
    this->publish_state(true);
    this->parent_->unlock();
    this->parent_->schedule_switch_off(AUTO_SWITCH_OFF_DELAY_MS);
  } else {
    // Allow manual turn-off
    this->publish_state(false);
  }
}

}  // namespace golmar_uno
}  // namespace esphome
