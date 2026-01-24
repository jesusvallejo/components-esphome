#include "wmbus.h"
#include "version.h"

#include "meters.h"

#include "address.h"

#include "esphome/core/application.h"
#include "esphome/core/helpers.h"

#ifdef USE_ESP_IDF
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_efuse.h"
#include "esp_mac.h"
#define GET_MILLIS() (xTaskGetTickCount() * portTICK_PERIOD_MS)
#else
#define GET_MILLIS() millis()
#endif

#ifdef USE_CAPTIVE_PORTAL
#include "esphome/components/captive_portal/captive_portal.h"
#endif

// Note: Stack size increase only works with Arduino framework
// For ESP-IDF, configure stack size in sdkconfig or menuconfig
#if defined(USE_ESP32) && !defined(USE_ESP_IDF) && defined(SET_LOOP_TASK_STACK_SIZE)
SET_LOOP_TASK_STACK_SIZE(32 * 1024);
#pragma message ( "Loop task stack increased." )
#endif

#ifdef USE_ESP8266
#error "ESP8266 not supported. Please use version 3.x: https://github.com/SzczepanLeon/esphome-components/issues/131"
#endif

namespace esphome {
namespace wmbus {

  static const char *TAG = "wmbus";

  void InfoComponent::setup() {
    return;
  }

  void WMBusComponent::setup() {
    this->high_freq_.start();
    if (this->led_pin_ != nullptr) {
      this->led_pin_->setup();
      this->led_pin_->digital_write(false);
      this->led_on_ = false;
    }
    if (!rf_mbus_.init(this->spi_conf_.mosi->get_pin(), this->spi_conf_.miso->get_pin(),
                       this->spi_conf_.clk->get_pin(),  this->spi_conf_.cs->get_pin(),
                       this->spi_conf_.gdo0->get_pin(), this->spi_conf_.gdo2->get_pin(),
                       this->frequency_, this->sync_mode_)) {
      this->mark_failed();
      ESP_LOGE(TAG, "RF chip initialization failed");
      return;
    }
  }

  void WMBusComponent::loop() {
    this->led_handler();
    if (rf_mbus_.task()) {
      ESP_LOGVV(TAG, "Have data from RF ...");
      WMbusFrame mbus_data = rf_mbus_.get_frame();

      std::string telegram = format_hex_pretty(mbus_data.frame);
      telegram.erase(std::remove(telegram.begin(), telegram.end(), '.'), telegram.end());

      this->frame_timestamp_ = this->time_->timestamp_now();
      Telegram t;
      if (t.parseHeader(mbus_data.frame) && t.addresses.empty()) {
        ESP_LOGE(TAG, "Address is empty! T: %s", telegram.c_str());
      }
      else {
        uint32_t meter_id = (uint32_t)strtoul(t.addresses[0].id.c_str(), nullptr, 16);
        bool meter_in_config = (this->wmbus_listeners_.count(meter_id) == 1) ? true : false;
        
        if (this->log_all_ || meter_in_config) {

          auto detected_drv_info      = pickMeterDriver(&t);
          std::string detected_driver = (detected_drv_info.name().str().empty() ? "" : detected_drv_info.name().str().c_str());

          auto used_drv_info      = detected_drv_info;
          std::string used_driver = detected_driver;
          if (meter_in_config) {
            auto *sensor = this->wmbus_listeners_[meter_id];
            used_driver = ((sensor->type).empty() ? detected_driver : sensor->type);
            if (!(sensor->type).empty()){
              auto *used_drv_info_ptr = lookupDriver(used_driver);
              if (used_drv_info_ptr == nullptr) {
                used_driver = detected_driver;
                used_drv_info = detected_drv_info;
                ESP_LOGW(TAG, "Selected driver %s doesn't exist, using %s", (sensor->type).c_str(), used_driver.c_str());
              }
              else{
                used_drv_info = *used_drv_info_ptr;
                ESP_LOGI(TAG, "Using selected driver %s (detected driver was %s)", used_driver.c_str(), detected_driver.c_str());
              }
            }
          }

          this->led_blink();
          ESP_LOGI(TAG, "%s [0x%08x] RSSI: %ddBm T: %s %c1 %c",
                    (used_driver.empty()? "Unknown!" : used_driver.c_str()),
                    meter_id,
                    mbus_data.rssi,
                    telegram.c_str(),
                    mbus_data.mode,
                    mbus_data.block);

          if (meter_in_config) {
            bool supported_link_mode{false};
            if (used_drv_info.linkModes().empty()) {
              supported_link_mode = true;
              ESP_LOGW(TAG, "Link modes not defined in driver %s. Processing anyway.",
                      (used_driver.empty()? "Unknown!" : used_driver.c_str()));
            }
            else {
              supported_link_mode = ( ((mbus_data.mode == 'T') && (used_drv_info.linkModes().has(LinkMode::T1))) ||
                                      ((mbus_data.mode == 'C') && (used_drv_info.linkModes().has(LinkMode::C1))) );
            }

            if (used_driver.empty()) {
              ESP_LOGW(TAG, "Can't find driver for T: %s", telegram.c_str());
            }
            else if (!supported_link_mode) {
              ESP_LOGW(TAG, "Link mode %c1 not supported in driver %s",
                      mbus_data.mode,
                      used_driver.c_str());
            }
            else {
              auto *sensor = this->wmbus_listeners_[meter_id];
              
              bool id_match;
              MeterInfo mi;
              mi.parse("ESPHome", used_driver, t.addresses[0].id + ",", sensor->myKey);
              auto meter = createMeter(&mi);
              std::vector<Address> addresses;
              AboutTelegram about{"ESPHome wM-Bus", mbus_data.rssi, FrameType::WMBUS, this->frame_timestamp_};
              meter->handleTelegram(about, mbus_data.frame, false, &addresses, &id_match, &t);
              if (id_match) {
                for (auto const& field : sensor->fields) {
                  std::string field_name = field.first.first;
                  std::string unit = field.first.second;
                  if (field_name == "rssi") {
                    field.second->publish_state(mbus_data.rssi);
                  }
                  else if (field.second->get_unit_of_measurement_ref().empty()) {
                    ESP_LOGW(TAG, "Fields without unit not supported as sensor, please switch to text_sensor.");
                  }
                  else {
                    Unit field_unit = toUnit(field.second->get_unit_of_measurement_ref());
                    if (field_unit != Unit::Unknown) {
                      double value  = meter->getNumericValue(field_name, field_unit);
                      if (!std::isnan(value)) {
                        field.second->publish_state(value);
                      }
                      else {
                        ESP_LOGW(TAG, "Can't get requested field '%s' with unit '%s'", field_name.c_str(), unit.c_str());
                      }
                    }
                    else {
                      ESP_LOGW(TAG, "Can't get proper unit from '%s'", field.second->get_unit_of_measurement_ref().c_str());
                    }
                  }
                }
                for (auto const& field : sensor->text_fields) {
                  if (meter->hasStringValue(field.first)) {
                    std::string value  = meter->getMyStringValue(field.first);
                    field.second->publish_state(value);
                  }
                  else {
                    ESP_LOGW(TAG, "Can't get requested field '%s'", field.first.c_str());
                  }
                }
              }
              else {
                ESP_LOGE(TAG, "Not for me T: %s", telegram.c_str());
              }
            }
          }
        }
      }
    }
  }

  void WMBusComponent::register_wmbus_listener(const uint32_t meter_id, const std::string type, const std::string key) {
    if (this->wmbus_listeners_.count(meter_id) == 0) {
      WMBusListener *listener = new wmbus::WMBusListener(meter_id, type, key);
      this->wmbus_listeners_.insert({meter_id, listener});
    }
  }

  void WMBusComponent::led_blink() {
    if (this->led_pin_ != nullptr) {
      if (!this->led_on_) {
        this->led_on_millis_ = GET_MILLIS();
        this->led_pin_->digital_write(true);
        this->led_on_ = true;
      }
    }
  }

  void WMBusComponent::led_handler() {
    if (this->led_pin_ != nullptr) {
      if (this->led_on_) {
        if ((GET_MILLIS() - this->led_on_millis_) >= this->led_blink_time_) {
          this->led_pin_->digital_write(false);
          this->led_on_ = false;
        }
      }
    }
  }

  void WMBusComponent::dump_config() {
    ESP_LOGCONFIG(TAG, "wM-Bus v%s-%s:", MY_VERSION, WMBUSMETERS_VERSION);
    if (this->led_pin_ != nullptr) {
      ESP_LOGCONFIG(TAG, "  LED:");
      LOG_PIN("    Pin: ", this->led_pin_);
      ESP_LOGCONFIG(TAG, "    Duration: %d ms", this->led_blink_time_);
    }
#ifdef USE_ESP32
#ifdef USE_ESP_IDF
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    ESP_LOGCONFIG(TAG, "  Chip ID: %02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#else
    ESP_LOGCONFIG(TAG, "  Chip ID: %012llX", ESP.getEfuseMac());
#endif
#endif
    ESP_LOGCONFIG(TAG, "  CC1101 frequency: %3.3f MHz", this->frequency_);
    ESP_LOGCONFIG(TAG, "  CC1101 SPI bus:");
    if (this->is_failed()) {
      ESP_LOGE(TAG, "   Check connection to CC1101!");
    }
    LOG_PIN("    MOSI Pin: ", this->spi_conf_.mosi);
    LOG_PIN("    MISO Pin: ", this->spi_conf_.miso);
    LOG_PIN("    CLK Pin:  ", this->spi_conf_.clk);
    LOG_PIN("    CS Pin:   ", this->spi_conf_.cs);
    LOG_PIN("    GDO0 Pin: ", this->spi_conf_.gdo0);
    LOG_PIN("    GDO2 Pin: ", this->spi_conf_.gdo2);
    std::string drivers = "";
    for (DriverInfo* p : allDrivers()) {
      drivers += p->name().str() + ", ";
    }
    drivers.erase(drivers.size() - 2);
    ESP_LOGCONFIG(TAG, "  Available drivers: %s", drivers.c_str());
    for (const auto &ele : this->wmbus_listeners_) {
      ele.second->dump_config();
    }
  }

  ///////////////////////////////////////

  void WMBusListener::dump_config() {
    std::string key = format_hex_pretty(this->key);
    key.erase(std::remove(key.begin(), key.end(), '.'), key.end());
    if (key.size()) {
      key.erase(key.size() - 5);
    }
    ESP_LOGCONFIG(TAG, "  Meter:");
    ESP_LOGCONFIG(TAG, "    ID: %zu [0x%08X]", this->id, this->id);
    ESP_LOGCONFIG(TAG, "    Type: %s", ((this->type).empty() ? "auto detect" : this->type.c_str()));
    ESP_LOGCONFIG(TAG, "    Key: '%s'", key.c_str());
    for (const auto &ele : this->fields) {
      ESP_LOGCONFIG(TAG, "    Field: '%s'", ele.first.first.c_str());
      LOG_SENSOR("     ", "Name:", ele.second);
    }
    for (const auto &ele : this->text_fields) {
      ESP_LOGCONFIG(TAG, "    Text field: '%s'", ele.first.c_str());
      LOG_TEXT_SENSOR("     ", "Name:", ele.second);
    }
  }

  WMBusListener::WMBusListener(const uint32_t id, const std::string type, const std::string key) {
    this->id = id;
    this->type = type;
    this->myKey = key;
    hex_to_bin(key, &(this->key));
  }

  int WMBusListener::char_to_int(char input)
  {
    if(input >= '0' && input <= '9') {
      return input - '0';
    }
    if(input >= 'A' && input <= 'F') {
      return input - 'A' + 10;
    }
    if(input >= 'a' && input <= 'f') {
      return input - 'a' + 10;
    }
    return -1;
  }

  bool WMBusListener::hex_to_bin(const char* src, std::vector<unsigned char> *target)
  {
    if (!src) return false;
    while(*src && src[1]) {
      if (*src == ' ' || *src == '#' || *src == '|' || *src == '_') {
        // Ignore space and hashes and pipes and underlines.
        src++;
      }
      else {
        int hi = char_to_int(*src);
        int lo = char_to_int(src[1]);
        if (hi<0 || lo<0) return false;
        target->push_back(hi*16 + lo);
        src += 2;
      }
    }
    return true;
  }

}  // namespace wmbus
}  // namespace esphome
