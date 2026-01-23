#include "cc1101.h"
#include "esphome/core/log.h"

#ifdef USE_ESP_IDF
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "freertos/task.h"
#endif

namespace esphome {
namespace wmbus {

static const char *TAG = "cc1101";

// Global instance
CC1101 cc1101;

#ifdef USE_ESP_IDF

bool CC1101::init_spi(uint8_t mosi, uint8_t miso, uint8_t clk, uint8_t cs) {
  this->mosi_pin_ = mosi;
  this->miso_pin_ = miso;
  this->clk_pin_ = clk;
  this->cs_pin_ = cs;

  // Configure SPI bus
  spi_bus_config_t buscfg = {};
  buscfg.mosi_io_num = mosi;
  buscfg.miso_io_num = miso;
  buscfg.sclk_io_num = clk;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;
  buscfg.max_transfer_sz = 64;

  // Initialize SPI bus on SPI2_HOST (HSPI)
  esp_err_t ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_DISABLED);
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
    return false;
  }

  // Configure SPI device
  spi_device_interface_config_t devcfg = {};
  devcfg.clock_speed_hz = 4000000;  // 4 MHz
  devcfg.mode = 0;                   // SPI mode 0
  devcfg.spics_io_num = cs;
  devcfg.queue_size = 1;
  devcfg.pre_cb = nullptr;
  devcfg.post_cb = nullptr;

  spi_device_handle_t spi;
  ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add SPI device: %s", esp_err_to_name(ret));
    return false;
  }

  this->spi_handle_ = spi;
  this->spi_initialized_ = true;
  
  ESP_LOGD(TAG, "SPI initialized: MOSI=%d, MISO=%d, CLK=%d, CS=%d", mosi, miso, clk, cs);
  return true;
}

void CC1101::spi_begin_() {
  // CS is handled by ESP-IDF SPI driver
}

void CC1101::spi_end_() {
  // CS is handled by ESP-IDF SPI driver  
}

uint8_t CC1101::spi_transfer_(uint8_t data) {
  if (!this->spi_initialized_ || this->spi_handle_ == nullptr) {
    return 0;
  }

  spi_device_handle_t spi = static_cast<spi_device_handle_t>(this->spi_handle_);
  
  spi_transaction_t t = {};
  t.length = 8;
  t.tx_buffer = &data;
  t.rx_buffer = nullptr;
  t.flags = SPI_TRANS_USE_RXDATA;

  esp_err_t ret = spi_device_polling_transmit(spi, &t);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "SPI transfer failed");
    return 0;
  }

  return t.rx_data[0];
}

void CC1101::wait_miso_low_() {
  // Wait for MISO to go low (chip ready)
  gpio_set_direction(static_cast<gpio_num_t>(this->miso_pin_), GPIO_MODE_INPUT);
  int timeout = 100;
  while (gpio_get_level(static_cast<gpio_num_t>(this->miso_pin_)) && timeout > 0) {
    esp_rom_delay_us(10);
    timeout--;
  }
}

#else  // Arduino framework fallback

bool CC1101::init_spi(uint8_t mosi, uint8_t miso, uint8_t clk, uint8_t cs) {
  this->mosi_pin_ = mosi;
  this->miso_pin_ = miso;
  this->clk_pin_ = clk;
  this->cs_pin_ = cs;
  
  // For Arduino, we use software SPI for maximum compatibility
  pinMode(mosi, OUTPUT);
  pinMode(miso, INPUT);
  pinMode(clk, OUTPUT);
  pinMode(cs, OUTPUT);
  digitalWrite(cs, HIGH);
  
  this->spi_initialized_ = true;
  return true;
}

void CC1101::spi_begin_() {
  digitalWrite(this->cs_pin_, LOW);
}

void CC1101::spi_end_() {
  digitalWrite(this->cs_pin_, HIGH);
}

uint8_t CC1101::spi_transfer_(uint8_t data) {
  // Software SPI implementation
  uint8_t result = 0;
  for (int i = 7; i >= 0; i--) {
    digitalWrite(this->clk_pin_, LOW);
    digitalWrite(this->mosi_pin_, (data >> i) & 0x01);
    delayMicroseconds(1);
    digitalWrite(this->clk_pin_, HIGH);
    result = (result << 1) | digitalRead(this->miso_pin_);
    delayMicroseconds(1);
  }
  digitalWrite(this->clk_pin_, LOW);
  return result;
}

void CC1101::wait_miso_low_() {
  int timeout = 100;
  while (digitalRead(this->miso_pin_) && timeout > 0) {
    delayMicroseconds(10);
    timeout--;
  }
}

#endif  // USE_ESP_IDF

bool CC1101::init() {
  // Reset the CC1101
  reset();
  
  // Verify communication by reading version
  uint8_t version = get_version();
  if (version == 0 || version == 0xFF) {
    ESP_LOGE(TAG, "CC1101 not found (version: 0x%02X)", version);
    return false;
  }
  
  ESP_LOGD(TAG, "CC1101 found (version: 0x%02X)", version);
  return true;
}

void CC1101::reset() {
  if (!this->spi_initialized_) return;

#ifdef USE_ESP_IDF
  // Manual CS control for reset sequence
  gpio_set_direction(static_cast<gpio_num_t>(this->cs_pin_), GPIO_MODE_OUTPUT);
  gpio_set_level(static_cast<gpio_num_t>(this->cs_pin_), 1);
  esp_rom_delay_us(5);
  gpio_set_level(static_cast<gpio_num_t>(this->cs_pin_), 0);
  esp_rom_delay_us(10);
  gpio_set_level(static_cast<gpio_num_t>(this->cs_pin_), 1);
  esp_rom_delay_us(45);
  gpio_set_level(static_cast<gpio_num_t>(this->cs_pin_), 0);
  
  wait_miso_low_();
  
  // Send SRES strobe
  spi_device_handle_t spi = static_cast<spi_device_handle_t>(this->spi_handle_);
  uint8_t cmd = CC1101_SRES;
  spi_transaction_t t = {};
  t.length = 8;
  t.tx_buffer = &cmd;
  spi_device_polling_transmit(spi, &t);
  
  gpio_set_level(static_cast<gpio_num_t>(this->cs_pin_), 1);
  
  // Wait for chip to be ready
  vTaskDelay(pdMS_TO_TICKS(1));
#else
  spi_begin_();
  wait_miso_low_();
  spi_transfer_(CC1101_SRES);
  spi_end_();
  delay(1);
#endif
}

void CC1101::write_reg(uint8_t addr, uint8_t value) {
  if (!this->spi_initialized_) return;

#ifdef USE_ESP_IDF
  spi_device_handle_t spi = static_cast<spi_device_handle_t>(this->spi_handle_);
  uint8_t tx_data[2] = {static_cast<uint8_t>(addr | CC1101_WRITE_SINGLE), value};
  
  spi_transaction_t t = {};
  t.length = 16;
  t.tx_buffer = tx_data;
  spi_device_polling_transmit(spi, &t);
#else
  spi_begin_();
  wait_miso_low_();
  spi_transfer_(addr | CC1101_WRITE_SINGLE);
  spi_transfer_(value);
  spi_end_();
#endif
}

void CC1101::write_burst(uint8_t addr, const uint8_t *buffer, uint8_t length) {
  if (!this->spi_initialized_ || buffer == nullptr || length == 0) return;

#ifdef USE_ESP_IDF
  spi_device_handle_t spi = static_cast<spi_device_handle_t>(this->spi_handle_);
  
  uint8_t *tx_data = new uint8_t[length + 1];
  tx_data[0] = addr | CC1101_WRITE_BURST;
  memcpy(tx_data + 1, buffer, length);
  
  spi_transaction_t t = {};
  t.length = (length + 1) * 8;
  t.tx_buffer = tx_data;
  spi_device_polling_transmit(spi, &t);
  
  delete[] tx_data;
#else
  spi_begin_();
  wait_miso_low_();
  spi_transfer_(addr | CC1101_WRITE_BURST);
  for (uint8_t i = 0; i < length; i++) {
    spi_transfer_(buffer[i]);
  }
  spi_end_();
#endif
}

uint8_t CC1101::read_reg(uint8_t addr) {
  if (!this->spi_initialized_) return 0;

#ifdef USE_ESP_IDF
  spi_device_handle_t spi = static_cast<spi_device_handle_t>(this->spi_handle_);
  
  uint8_t tx_data[2] = {static_cast<uint8_t>(addr | CC1101_READ_SINGLE), 0x00};
  uint8_t rx_data[2] = {0};
  
  spi_transaction_t t = {};
  t.length = 16;
  t.tx_buffer = tx_data;
  t.rx_buffer = rx_data;
  spi_device_polling_transmit(spi, &t);
  
  return rx_data[1];
#else
  uint8_t value;
  spi_begin_();
  wait_miso_low_();
  spi_transfer_(addr | CC1101_READ_SINGLE);
  value = spi_transfer_(0x00);
  spi_end_();
  return value;
#endif
}

uint8_t CC1101::read_status(uint8_t addr) {
  if (!this->spi_initialized_) return 0;

#ifdef USE_ESP_IDF
  spi_device_handle_t spi = static_cast<spi_device_handle_t>(this->spi_handle_);
  
  uint8_t tx_data[2] = {static_cast<uint8_t>(addr | CC1101_READ_BURST), 0x00};
  uint8_t rx_data[2] = {0};
  
  spi_transaction_t t = {};
  t.length = 16;
  t.tx_buffer = tx_data;
  t.rx_buffer = rx_data;
  spi_device_polling_transmit(spi, &t);
  
  return rx_data[1];
#else
  uint8_t value;
  spi_begin_();
  wait_miso_low_();
  spi_transfer_(addr | CC1101_READ_BURST);
  value = spi_transfer_(0x00);
  spi_end_();
  return value;
#endif
}

void CC1101::read_burst(uint8_t addr, uint8_t *buffer, uint8_t length) {
  if (!this->spi_initialized_ || buffer == nullptr || length == 0) return;

#ifdef USE_ESP_IDF
  spi_device_handle_t spi = static_cast<spi_device_handle_t>(this->spi_handle_);
  
  uint8_t *tx_data = new uint8_t[length + 1];
  uint8_t *rx_data = new uint8_t[length + 1];
  
  memset(tx_data, 0, length + 1);
  tx_data[0] = addr | CC1101_READ_BURST;
  
  spi_transaction_t t = {};
  t.length = (length + 1) * 8;
  t.tx_buffer = tx_data;
  t.rx_buffer = rx_data;
  spi_device_polling_transmit(spi, &t);
  
  memcpy(buffer, rx_data + 1, length);
  
  delete[] tx_data;
  delete[] rx_data;
#else
  spi_begin_();
  wait_miso_low_();
  spi_transfer_(addr | CC1101_READ_BURST);
  for (uint8_t i = 0; i < length; i++) {
    buffer[i] = spi_transfer_(0x00);
  }
  spi_end_();
#endif
}

uint8_t CC1101::strobe(uint8_t cmd) {
  if (!this->spi_initialized_) return 0;

#ifdef USE_ESP_IDF
  spi_device_handle_t spi = static_cast<spi_device_handle_t>(this->spi_handle_);
  
  spi_transaction_t t = {};
  t.length = 8;
  t.tx_buffer = &cmd;
  t.flags = SPI_TRANS_USE_RXDATA;
  spi_device_polling_transmit(spi, &t);
  
  return t.rx_data[0];
#else
  uint8_t status;
  spi_begin_();
  wait_miso_low_();
  status = spi_transfer_(cmd);
  spi_end_();
  return status;
#endif
}

void CC1101::set_rx() {
  strobe(CC1101_SRX);
}

void CC1101::set_idle() {
  strobe(CC1101_SIDLE);
}

int8_t CC1101::get_rssi() {
  uint8_t rssi_raw = read_status(CC1101_RSSI);
  int16_t rssi;
  
  if (rssi_raw >= 128) {
    rssi = (int16_t)((int16_t)(rssi_raw - 256) / 2) - 74;
  } else {
    rssi = (rssi_raw / 2) - 74;
  }
  
  return (int8_t)rssi;
}

uint8_t CC1101::get_lqi() {
  return read_status(CC1101_LQI) & 0x7F;
}

uint8_t CC1101::get_version() {
  return read_status(CC1101_VERSION);
}

}  // namespace wmbus
}  // namespace esphome
