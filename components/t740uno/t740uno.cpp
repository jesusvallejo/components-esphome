#include "t740uno.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"


namespace esphome {
namespace t740uno {

static const char *const TAG = "t740uno";

void T740UNO::loop() {
    while (available()) {
   
    }
}



void T740UNO::dump_config() {
    ESP_LOGCONFIG(TAG, "T740 UNO V2");
  }

}  // namespace t740uno
}  // namespace esphome