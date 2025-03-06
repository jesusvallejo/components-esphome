#include "open_door_button.h"

namespace esphome {
namespace t740uno {

void OpenDoorButton::press_action() { this->parent_->open(); }

}  // namespace t740uno
}  // namespace esphome