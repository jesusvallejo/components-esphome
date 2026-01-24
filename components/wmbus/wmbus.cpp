#include "wmbus.h"
#include "version.h"
#include "meters.h"
#include "address.h"

#include "esphome/core/application.h"
#include "esphome/core/helpers.h"

#include "freertos/task.h"
#include "esp_log.h"
#include "esp_efuse.h"
#include "esp_mac.h"

#include <algorithm>
#include <cmath>

#ifdef USE_CAPTIVE_PORTAL
#include "esphome/components/captive_portal/captive_portal.h"
#endif

#ifdef USE_ESP8266
#error "ESP8266 not supported. Please use version 3.x"
#endif

namespace esphome {
namespace wmbus {

namespace {

static const char *TAG = "wmbus";

inline uint32_t get_millis() {
  return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

}  // namespace

void InfoComponent::setup() {
  // No initialization required
}

void WMBusComponent::setup() {
  high_freq_.start();

  if (led_pin_ != nullptr) {
    led_pin_->setup();
    led_pin_->digital_write(false);
    led_on_ = false;
  }

  if (!rf_mbus_.init(spi_conf_.mosi->get_pin(), spi_conf_.miso->get_pin(),
                     spi_conf_.clk->get_pin(), spi_conf_.cs->get_pin(),
                     spi_conf_.gdo0->get_pin(), spi_conf_.gdo2->get_pin(),
                     frequency_, sync_mode_)) {
    mark_failed();
    ESP_LOGE(TAG, "RF chip initialization failed");
    return;
  }
}

void WMBusComponent::loop() {
  led_handler();

  if (!rf_mbus_.task()) {
    return;
  }

  ESP_LOGVV(TAG, "Have data from RF...");
  WMbusFrame mbus_data = rf_mbus_.get_frame();

  // Format telegram for logging
  std::string telegram = format_hex_pretty(mbus_data.frame);
  telegram.erase(std::remove(telegram.begin(), telegram.end(), '.'), telegram.end());

  frame_timestamp_ = time_->timestamp_now();

  // Parse telegram header
  Telegram t;
  if (t.parseHeader(mbus_data.frame) && t.addresses.empty()) {
    ESP_LOGE(TAG, "Address is empty! T: %s", telegram.c_str());
    return;
  }

  uint32_t meter_id = static_cast<uint32_t>(strtoul(t.addresses[0].id.c_str(), nullptr, 16));
  bool meter_in_config = wmbus_listeners_.count(meter_id) > 0;

  if (!log_all_ && !meter_in_config) {
    return;
  }

  // Detect driver
  auto detected_drv_info = pickMeterDriver(&t);
  std::string detected_driver = detected_drv_info.name().str();

  auto used_drv_info = detected_drv_info;
  std::string used_driver = detected_driver;

  // Override driver if configured
  if (meter_in_config) {
    auto *listener = wmbus_listeners_[meter_id];
    if (!listener->type.empty()) {
      auto *configured_drv = lookupDriver(listener->type);
      if (configured_drv != nullptr) {
        used_drv_info = *configured_drv;
        used_driver = listener->type;
        ESP_LOGI(TAG, "Using configured driver %s (detected: %s)", 
                 used_driver.c_str(), detected_driver.c_str());
      } else {
        ESP_LOGW(TAG, "Configured driver %s doesn't exist, using %s", 
                 listener->type.c_str(), used_driver.c_str());
      }
    }
  }

  led_blink();
  ESP_LOGI(TAG, "%s [0x%08X] RSSI: %ddBm T: %s %c1 %c",
           used_driver.empty() ? "Unknown!" : used_driver.c_str(),
           meter_id, mbus_data.rssi, telegram.c_str(),
           mbus_data.mode, mbus_data.block);

  if (!meter_in_config) {
    return;
  }

  // Validate driver and link mode
  if (used_driver.empty()) {
    ESP_LOGW(TAG, "Can't find driver for T: %s", telegram.c_str());
    return;
  }

  bool link_mode_supported = used_drv_info.linkModes().empty();
  if (!link_mode_supported) {
    link_mode_supported = (mbus_data.mode == 'T' && used_drv_info.linkModes().has(LinkMode::T1)) ||
                          (mbus_data.mode == 'C' && used_drv_info.linkModes().has(LinkMode::C1));
  } else {
    ESP_LOGW(TAG, "Link modes not defined in driver %s, processing anyway", used_driver.c_str());
  }

  if (!link_mode_supported) {
    ESP_LOGW(TAG, "Link mode %c1 not supported in driver %s", mbus_data.mode, used_driver.c_str());
    return;
  }

  // Process telegram
  auto *listener = wmbus_listeners_[meter_id];
  
  MeterInfo mi;
  mi.parse("ESPHome", used_driver, t.addresses[0].id + ",", listener->myKey);
  auto meter = createMeter(&mi);

  std::vector<Address> addresses;
  bool id_match = false;
  AboutTelegram about{"ESPHome wM-Bus", mbus_data.rssi, FrameType::WMBUS, frame_timestamp_};
  meter->handleTelegram(about, mbus_data.frame, false, &addresses, &id_match, &t);

  if (!id_match) {
    ESP_LOGE(TAG, "ID mismatch for T: %s", telegram.c_str());
    return;
  }

  // Publish sensor values
  for (const auto &[field_key, sensor] : listener->fields) {
    const std::string &field_name = field_key.first;
    const std::string &unit = field_key.second;

    if (field_name == "rssi") {
      sensor->publish_state(mbus_data.rssi);
      continue;
    }

    const std::string &sensor_unit = sensor->get_unit_of_measurement_ref();
    if (sensor_unit.empty()) {
      ESP_LOGW(TAG, "Field '%s' has no unit, use text_sensor instead", field_name.c_str());
      continue;
    }

    Unit field_unit = toUnit(sensor_unit);
    if (field_unit == Unit::Unknown) {
      ESP_LOGW(TAG, "Unknown unit '%s' for field '%s'", sensor_unit.c_str(), field_name.c_str());
      continue;
    }

    double value = meter->getNumericValue(field_name, field_unit);
    if (!std::isnan(value)) {
      sensor->publish_state(value);
    } else {
      ESP_LOGW(TAG, "Can't get field '%s' with unit '%s'", field_name.c_str(), unit.c_str());
    }
  }

  // Publish text sensor values
  for (const auto &[field_name, text_sensor] : listener->text_fields) {
    if (meter->hasStringValue(field_name)) {
      text_sensor->publish_state(meter->getMyStringValue(field_name));
    } else {
      ESP_LOGW(TAG, "Can't get text field '%s'", field_name.c_str());
    }
  }
}

void WMBusComponent::register_wmbus_listener(uint32_t meter_id, const std::string &type, 
                                              const std::string &key) {
  if (wmbus_listeners_.count(meter_id) == 0) {
    auto *listener = new WMBusListener(meter_id, type, key);
    wmbus_listeners_.insert({meter_id, listener});
  }
}

void WMBusComponent::led_blink() {
  if (led_pin_ == nullptr || led_on_) {
    return;
  }
  led_on_millis_ = get_millis();
  led_pin_->digital_write(true);
  led_on_ = true;
}

void WMBusComponent::led_handler() {
  if (led_pin_ == nullptr || !led_on_) {
    return;
  }
  if ((get_millis() - led_on_millis_) >= led_blink_time_) {
    led_pin_->digital_write(false);
    led_on_ = false;
  }
}

void WMBusComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "wM-Bus v%s-%s:", MY_VERSION, WMBUSMETERS_VERSION);

  if (led_pin_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  LED:");
    LOG_PIN("    Pin: ", led_pin_);
    ESP_LOGCONFIG(TAG, "    Duration: %d ms", led_blink_time_);
  }

#ifdef USE_ESP32
  uint8_t mac[6];
  esp_efuse_mac_get_default(mac);
  ESP_LOGCONFIG(TAG, "  Chip ID: %02X%02X%02X%02X%02X%02X", 
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif

  ESP_LOGCONFIG(TAG, "  CC1101 frequency: %.3f MHz", frequency_);
  ESP_LOGCONFIG(TAG, "  CC1101 SPI bus:");

  if (is_failed()) {
    ESP_LOGE(TAG, "   Check connection to CC1101!");
  }

  LOG_PIN("    MOSI Pin: ", spi_conf_.mosi);
  LOG_PIN("    MISO Pin: ", spi_conf_.miso);
  LOG_PIN("    CLK Pin:  ", spi_conf_.clk);
  LOG_PIN("    CS Pin:   ", spi_conf_.cs);
  LOG_PIN("    GDO0 Pin: ", spi_conf_.gdo0);
  LOG_PIN("    GDO2 Pin: ", spi_conf_.gdo2);

  // Build driver list
  std::string drivers;
  for (DriverInfo *driver : allDrivers()) {
    if (!drivers.empty()) {
      drivers += ", ";
    }
    drivers += driver->name().str();
  }
  ESP_LOGCONFIG(TAG, "  Available drivers: %s", drivers.c_str());

  for (const auto &[id, listener] : wmbus_listeners_) {
    listener->dump_config();
  }
}

// =============================================================================
// WMBusListener Implementation
// =============================================================================

WMBusListener::WMBusListener(uint32_t id, const std::string &type, const std::string &key)
    : id(id), type(type), myKey(key) {
  hex_to_bin(key.c_str(), &this->key);
}

void WMBusListener::dump_config() {
  std::string key_str = format_hex_pretty(key);
  key_str.erase(std::remove(key_str.begin(), key_str.end(), '.'), key_str.end());
  if (!key_str.empty() && key_str.size() >= 5) {
    key_str.erase(key_str.size() - 5);
  }

  ESP_LOGCONFIG(TAG, "  Meter:");
  ESP_LOGCONFIG(TAG, "    ID: %u [0x%08X]", id, id);
  ESP_LOGCONFIG(TAG, "    Type: %s", type.empty() ? "auto detect" : type.c_str());
  ESP_LOGCONFIG(TAG, "    Key: '%s'", key_str.c_str());

  for (const auto &[field_key, sensor] : fields) {
    ESP_LOGCONFIG(TAG, "    Field: '%s'", field_key.first.c_str());
    LOG_SENSOR("     ", "Name:", sensor);
  }

  for (const auto &[field_name, text_sensor] : text_fields) {
    ESP_LOGCONFIG(TAG, "    Text field: '%s'", field_name.c_str());
    LOG_TEXT_SENSOR("     ", "Name:", text_sensor);
  }
}

int WMBusListener::char_to_int(char input) {
  if (input >= '0' && input <= '9') {
    return input - '0';
  }
  if (input >= 'A' && input <= 'F') {
    return input - 'A' + 10;
  }
  if (input >= 'a' && input <= 'f') {
    return input - 'a' + 10;
  }
  return -1;
}

bool WMBusListener::hex_to_bin(const char *src, std::vector<unsigned char> *target) {
  if (src == nullptr || target == nullptr) {
    return false;
  }

  while (*src && src[1]) {
    // Skip whitespace and separators
    if (*src == ' ' || *src == '#' || *src == '|' || *src == '_') {
      ++src;
      continue;
    }

    int hi = char_to_int(*src);
    int lo = char_to_int(src[1]);
    if (hi < 0 || lo < 0) {
      return false;
    }

    target->push_back(static_cast<unsigned char>(hi * 16 + lo));
    src += 2;
  }

  return true;
}

}  // namespace wmbus
}  // namespace esphome
