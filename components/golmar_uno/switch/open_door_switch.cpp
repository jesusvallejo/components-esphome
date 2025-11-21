#include "open_door_switch.h"
#include "esphome/core/log.h"

namespace esphome {
namespace golmar_uno {

static const char *const TAG = "golmar_uno.switch";

void open_door_switch::write_state(bool state) {
    if (state) {
        ESP_LOGD(TAG, "Opening door via switch");
        this->parent_->open();
        // Auto-turn off after 2 seconds
        this->parent_->set_timeout(2000, [this]() {
            this->publish_state(false);
        });
    }
}

}  // namespace golmar_uno
}
