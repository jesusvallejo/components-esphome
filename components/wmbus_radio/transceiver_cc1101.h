#pragma once
#include "transceiver.h"

namespace esphome {
namespace wmbus_radio {
class CC1101 : public RadioTransceiver {
public:
  void setup() override;
  optional<uint8_t> read() override;
  void restart_rx() override;
  void clear_rx() override;
  int8_t get_rssi() override;
  const char *get_name() override;

  void set_frequency(float frequency_hz);
  void set_bitrate(uint32_t bitrate_bps);
  void set_deviation(float deviation_hz);
  void set_rx_bandwidth(float bandwidth_hz);
  void set_channel_spacing(float spacing_hz);

protected:
  uint8_t spi_read_reg_(uint8_t address);
  void spi_read_burst_(uint8_t address, uint8_t *data, size_t length);
  uint8_t spi_read_status_(uint8_t address);
  void spi_write_reg_(uint8_t address, uint8_t data);
  void spi_write_burst_(uint8_t address, std::initializer_list<uint8_t> data);
  void strobe_(uint8_t command);

  float frequency_hz_{868950000.0f};
  uint32_t bitrate_bps_{100000};
  float deviation_hz_{50000.0f};
  float rx_bandwidth_hz_{200000.0f};
  float channel_spacing_hz_{200000.0f};
  std::array<uint8_t, 64> fifo_cache_{};
  size_t fifo_cache_size_{0};
  size_t fifo_cache_index_{0};
  bool sync_seen_{false};
};
} // namespace wmbus_radio
} // namespace esphome
