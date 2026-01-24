#include "cc1101.h"
#include "esphome/core/log.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "freertos/task.h"

namespace esphome {
namespace wmbus {

static const char *TAG = "cc1101";

// SPI host selection based on chip variant
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C2 || CONFIG_IDF_TARGET_ESP32C6
static constexpr spi_host_device_t CC1101_SPI_HOST = SPI2_HOST;
#else
static constexpr spi_host_device_t CC1101_SPI_HOST = SPI2_HOST;  // HSPI on ESP32
#endif

// Timing constants (microseconds)
static constexpr uint32_t CS_SETUP_TIME_US = 1;
static constexpr uint32_t CS_HOLD_TIME_US = 1;
static constexpr uint32_t MISO_WAIT_TIME_US = 20;
static constexpr uint32_t RESET_DELAY_MS = 10;

// Reset sequence timing (microseconds)
static constexpr uint32_t RESET_CS_HIGH_US = 5;
static constexpr uint32_t RESET_CS_LOW_US = 10;
static constexpr uint32_t RESET_CS_WAIT_US = 45;

// Global instance
CC1101 cc1101;

bool CC1101::init_spi(uint8_t mosi, uint8_t miso, uint8_t clk, uint8_t cs) {
  mosi_pin_ = mosi;
  miso_pin_ = miso;
  clk_pin_ = clk;
  cs_pin_ = cs;

  ESP_LOGI(TAG, "Initializing SPI: MOSI=%d, MISO=%d, CLK=%d, CS=%d", mosi, miso, clk, cs);

  // Configure CS pin as GPIO output (manual control for proper reset sequence)
  const gpio_config_t cs_config = {
    .pin_bit_mask = (1ULL << cs),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };
  gpio_config(&cs_config);
  gpio_set_level(static_cast<gpio_num_t>(cs), 1);

  // Configure SPI bus
  const spi_bus_config_t bus_config = {
    .mosi_io_num = mosi,
    .miso_io_num = miso,
    .sclk_io_num = clk,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .data4_io_num = -1,
    .data5_io_num = -1,
    .data6_io_num = -1,
    .data7_io_num = -1,
    .max_transfer_sz = 64,
    .flags = SPICOMMON_BUSFLAG_MASTER,
    .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
    .intr_flags = 0,
  };

  esp_err_t ret = spi_bus_initialize(CC1101_SPI_HOST, &bus_config, SPI_DMA_DISABLED);
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
    return false;
  }
  ESP_LOGI(TAG, "SPI bus initialized");

  // Configure SPI device with manual CS control
  const spi_device_interface_config_t dev_config = {
    .command_bits = 0,
    .address_bits = 0,
    .dummy_bits = 0,
    .mode = 0,  // SPI mode 0 (CPOL=0, CPHA=0)
    .clock_source = SPI_CLK_SRC_DEFAULT,
    .duty_cycle_pos = 128,
    .cs_ena_pretrans = 0,
    .cs_ena_posttrans = 0,
    .clock_speed_hz = static_cast<int>(CC1101_SPI_CLOCK_HZ),
    .input_delay_ns = 0,
    .spics_io_num = -1,  // Manual CS control
    .flags = 0,
    .queue_size = 1,
    .pre_cb = nullptr,
    .post_cb = nullptr,
  };

  spi_device_handle_t spi_handle;
  ret = spi_bus_add_device(CC1101_SPI_HOST, &dev_config, &spi_handle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add SPI device: %s", esp_err_to_name(ret));
    return false;
  }

  spi_handle_ = spi_handle;
  spi_initialized_ = true;
  
  ESP_LOGI(TAG, "SPI device added successfully");
  return true;
}

void CC1101::spi_begin_() {
  gpio_set_level(static_cast<gpio_num_t>(cs_pin_), 0);
  esp_rom_delay_us(CS_SETUP_TIME_US);
}

void CC1101::spi_end_() {
  esp_rom_delay_us(CS_HOLD_TIME_US);
  gpio_set_level(static_cast<gpio_num_t>(cs_pin_), 1);
}

uint8_t CC1101::spi_transfer_(uint8_t data) {
  if (!spi_initialized_ || spi_handle_ == nullptr) {
    ESP_LOGE(TAG, "SPI not initialized");
    return 0;
  }

  auto *spi = static_cast<spi_device_handle_t>(spi_handle_);
  
  uint8_t rx_data = 0;
  spi_transaction_t trans = {
    .flags = 0,
    .cmd = 0,
    .addr = 0,
    .length = 8,
    .rxlength = 0,
    .user = nullptr,
    .tx_buffer = &data,
    .rx_buffer = &rx_data,
  };

  esp_err_t ret = spi_device_polling_transmit(spi, &trans);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "SPI transfer failed: %s", esp_err_to_name(ret));
    return 0;
  }

  return rx_data;
}

void CC1101::wait_miso_low_() {
  // Small delay to ensure CC1101 is ready
  // Note: MISO is controlled by the SPI peripheral, so we use a fixed delay
  // The CC1101 typically responds within a few microseconds
  esp_rom_delay_us(MISO_WAIT_TIME_US);
}

bool CC1101::init() {
  reset();
  
  uint8_t version = get_version();
  if (version == 0x00 || version == 0xFF) {
    ESP_LOGE(TAG, "CC1101 not found (version: 0x%02X)", version);
    return false;
  }
  
  ESP_LOGD(TAG, "CC1101 found (version: 0x%02X)", version);
  return true;
}

void CC1101::reset() {
  if (!spi_initialized_) {
    return;
  }

  ESP_LOGI(TAG, "Resetting CC1101...");

  const auto cs = static_cast<gpio_num_t>(cs_pin_);

  // CC1101 manual reset sequence (see datasheet)
  gpio_set_level(cs, 1);
  esp_rom_delay_us(RESET_CS_HIGH_US);
  gpio_set_level(cs, 0);
  esp_rom_delay_us(RESET_CS_LOW_US);
  gpio_set_level(cs, 1);
  esp_rom_delay_us(RESET_CS_WAIT_US);
  gpio_set_level(cs, 0);
  
  wait_miso_low_();
  spi_transfer_(CC1101_SRES);
  gpio_set_level(cs, 1);
  
  // Wait for chip to be ready after reset
  vTaskDelay(pdMS_TO_TICKS(RESET_DELAY_MS));
  
  ESP_LOGI(TAG, "CC1101 reset complete");
}

void CC1101::write_reg(uint8_t addr, uint8_t value) {
  if (!spi_initialized_) {
    return;
  }
  spi_begin_();
  wait_miso_low_();
  spi_transfer_(addr | CC1101_WRITE_SINGLE);
  spi_transfer_(value);
  spi_end_();
}

void CC1101::write_burst(uint8_t addr, const uint8_t *buffer, uint8_t length) {
  if (!spi_initialized_ || buffer == nullptr || length == 0) {
    return;
  }
  spi_begin_();
  wait_miso_low_();
  spi_transfer_(addr | CC1101_WRITE_BURST);
  for (uint8_t i = 0; i < length; ++i) {
    spi_transfer_(buffer[i]);
  }
  spi_end_();
}

uint8_t CC1101::read_reg(uint8_t addr) {
  if (!spi_initialized_) {
    return 0;
  }
  spi_begin_();
  wait_miso_low_();
  spi_transfer_(addr | CC1101_READ_SINGLE);
  uint8_t value = spi_transfer_(0x00);
  spi_end_();
  return value;
}

uint8_t CC1101::read_status(uint8_t addr) {
  if (!spi_initialized_) {
    return 0;
  }
  spi_begin_();
  wait_miso_low_();
  spi_transfer_(addr | CC1101_READ_BURST);
  uint8_t value = spi_transfer_(0x00);
  spi_end_();
  return value;
}

void CC1101::read_burst(uint8_t addr, uint8_t *buffer, uint8_t length) {
  if (!spi_initialized_ || buffer == nullptr || length == 0) {
    return;
  }
  spi_begin_();
  wait_miso_low_();
  spi_transfer_(addr | CC1101_READ_BURST);
  for (uint8_t i = 0; i < length; ++i) {
    buffer[i] = spi_transfer_(0x00);
  }
  spi_end_();
}

uint8_t CC1101::strobe(uint8_t cmd) {
  if (!spi_initialized_) {
    return 0;
  }
  spi_begin_();
  wait_miso_low_();
  uint8_t status = spi_transfer_(cmd);
  spi_end_();
  return status;
}

void CC1101::set_rx() {
  strobe(CC1101_SRX);
}

void CC1101::set_idle() {
  strobe(CC1101_SIDLE);
}

int8_t CC1101::get_rssi() {
  uint8_t rssi_raw = read_status(CC1101_RSSI);
  
  // Convert to signed value and calculate dBm
  // Formula from CC1101 datasheet: RSSI_dBm = (RSSI_raw / 2) - RSSI_OFFSET
  int16_t rssi_signed = (rssi_raw >= 128) ? static_cast<int16_t>(rssi_raw) - 256 
                                          : static_cast<int16_t>(rssi_raw);
  return static_cast<int8_t>((rssi_signed / 2) - CC1101_RSSI_OFFSET);
}

uint8_t CC1101::get_lqi() {
  // LQI is in bits 6:0, bit 7 is CRC_OK
  return read_status(CC1101_LQI) & 0x7F;
}

uint8_t CC1101::get_version() {
  return read_status(CC1101_VERSION);
}

}  // namespace wmbus
}  // namespace esphome
