#include "open_door_switch.h"
#include "esphome/core/log.h"

namespace esphome {
namespace golmar_uno {

static const char *const TAG = "golmar_uno.switch";

void unlock_door_switch::write_state(bool state) {
    if (state) {
        ESP_LOGD(TAG, "Unlocking door via switch");
        this->parent_->unlock();
        // Auto-turn off after 2 seconds via parent's schedule_switch_off method
        this->parent_->schedule_switch_off(2000);
    }
}

}  // namespace golmar_uno
}  // namespace esphome
