#include "open_door_button.h"
#include "esphome/core/log.h"

namespace esphome {
namespace golmar_uno {

static const char *const TAG = "golmar_uno.button";

void UnlockDoorButton::press_action() {
  ESP_LOGD(TAG, "Unlock button pressed");
  this->parent_->unlock();
}

}  // namespace golmar_uno
}  // namespace esphome