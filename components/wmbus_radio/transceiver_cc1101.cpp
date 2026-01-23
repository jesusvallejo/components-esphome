#include "transceiver_cc1101.h"

#include "esphome/core/log.h"

namespace esphome {
namespace wmbus_radio {
static const char *TAG = "CC1101";

namespace {
constexpr uint32_t F_XOSC = 26000000;
constexpr uint8_t REG_IOCFG0 = 0x02;
constexpr uint8_t REG_FIFOTHR = 0x03;
constexpr uint8_t REG_SYNC1 = 0x04;
constexpr uint8_t REG_SYNC0 = 0x05;
constexpr uint8_t REG_PKTLEN = 0x06;
constexpr uint8_t REG_PKTCTRL1 = 0x07;
constexpr uint8_t REG_PKTCTRL0 = 0x08;
constexpr uint8_t REG_FSCTRL1 = 0x0B;
constexpr uint8_t REG_FSCTRL0 = 0x0C;
constexpr uint8_t REG_FREQ2 = 0x0D;
constexpr uint8_t REG_MDMCFG4 = 0x10;
constexpr uint8_t REG_MDMCFG3 = 0x11;
constexpr uint8_t REG_MDMCFG2 = 0x12;
constexpr uint8_t REG_MDMCFG1 = 0x13;
constexpr uint8_t REG_MDMCFG0 = 0x14;
constexpr uint8_t REG_DEVIATN = 0x15;
constexpr uint8_t REG_RSSI = 0x34;
constexpr uint8_t REG_RXBYTES = 0x3B;

struct ModemConfig {
  uint8_t mdmcfg4;
  uint8_t mdmcfg3;
  uint8_t mdmcfg2;
  uint8_t mdmcfg1;
  uint8_t mdmcfg0;
  uint8_t deviatn;
};

uint32_t compute_frequency_word(uint32_t frequency) {
  return (uint32_t)(((uint64_t)frequency << 16) / F_XOSC);
}

void compute_drate(uint32_t bitrate, uint8_t &drate_e, uint8_t &drate_m) {
  uint32_t best_err = 0xFFFFFFFF;
  uint8_t best_e = 0;
  uint8_t best_m = 0;
  for (uint8_t e = 0; e <= 15; e++) {
    for (uint16_t m = 0; m <= 255; m++) {
      uint32_t rate = (uint32_t)((((uint64_t)F_XOSC) * (256 + m) * (1u << e)) >> 28);
      uint32_t err = rate > bitrate ? (rate - bitrate) : (bitrate - rate);
      if (err < best_err) {
        best_err = err;
        best_e = e;
        best_m = (uint8_t)m;
      }
      if (best_err == 0)
        break;
    }
    if (best_err == 0)
      break;
  }
  drate_e = best_e;
  drate_m = best_m;
}

void compute_deviation(uint32_t deviation, uint8_t &dev_e, uint8_t &dev_m) {
  uint32_t best_err = 0xFFFFFFFF;
  uint8_t best_e = 0;
  uint8_t best_m = 0;
  for (uint8_t e = 0; e <= 7; e++) {
    for (uint8_t m = 0; m <= 7; m++) {
      uint32_t dev = (uint32_t)((((uint64_t)F_XOSC) * (8 + m) * (1u << e)) >> 17);
      uint32_t err = dev > deviation ? (dev - deviation) : (deviation - dev);
      if (err < best_err) {
        best_err = err;
        best_e = e;
        best_m = m;
      }
      if (best_err == 0)
        break;
    }
    if (best_err == 0)
      break;
  }
  dev_e = best_e;
  dev_m = best_m;
}

void compute_rx_bw(uint32_t bandwidth, uint8_t &bw_e, uint8_t &bw_m) {
  uint32_t best_err = 0xFFFFFFFF;
  uint8_t best_e = 0;
  uint8_t best_m = 0;
  for (uint8_t e = 0; e <= 3; e++) {
    for (uint8_t m = 0; m <= 3; m++) {
      uint32_t bw = (uint32_t)(F_XOSC / (8 * (4 + m) * (1u << e)));
      uint32_t err = bw > bandwidth ? (bw - bandwidth) : (bandwidth - bw);
      if (err < best_err) {
        best_err = err;
        best_e = e;
        best_m = m;
      }
      if (best_err == 0)
        break;
    }
    if (best_err == 0)
      break;
  }
  bw_e = best_e;
  bw_m = best_m;
}

void compute_channel_spacing(uint32_t spacing, uint8_t &cs_e, uint8_t &cs_m) {
  uint32_t best_err = 0xFFFFFFFF;
  uint8_t best_e = 0;
  uint8_t best_m = 0;
  for (uint8_t e = 0; e <= 3; e++) {
    for (uint16_t m = 0; m <= 255; m++) {
      uint32_t sp = (uint32_t)((((uint64_t)F_XOSC) * (256 + m) * (1u << e)) >> 18);
      uint32_t err = sp > spacing ? (sp - spacing) : (spacing - sp);
      if (err < best_err) {
        best_err = err;
        best_e = e;
        best_m = (uint8_t)m;
      }
      if (best_err == 0)
        break;
    }
    if (best_err == 0)
      break;
  }
  cs_e = best_e;
  cs_m = best_m;
}

ModemConfig build_modem_config(uint32_t bitrate, uint32_t deviation,
                               uint32_t rx_bandwidth,
                               uint32_t channel_spacing) {
  uint8_t drate_e = 0;
  uint8_t drate_m = 0;
  uint8_t dev_e = 0;
  uint8_t dev_m = 0;
  uint8_t bw_e = 0;
  uint8_t bw_m = 0;
  uint8_t cs_e = 0;
  uint8_t cs_m = 0;

  compute_drate(bitrate, drate_e, drate_m);
  compute_deviation(deviation, dev_e, dev_m);
  compute_rx_bw(rx_bandwidth, bw_e, bw_m);
  compute_channel_spacing(channel_spacing, cs_e, cs_m);

  ModemConfig config{};
  config.mdmcfg4 = (bw_e << 6) | (bw_m << 4) | (drate_e & 0x0F);
  config.mdmcfg3 = drate_m;
  config.mdmcfg2 = (1u << 4) | 0x02; // 2-FSK, 16/16 sync word
  config.mdmcfg1 = (2u << 4) | (cs_e & 0x03); // 4 preamble bytes
  config.mdmcfg0 = cs_m;
  config.deviatn = (dev_e << 4) | (dev_m & 0x07);
  return config;
}
} // namespace

void CC1101::setup() {
  this->common_setup();

  ESP_LOGV(TAG, "Setup");
  this->reset();
  this->strobe_(0x30); // SRES
  delay(5);

  uint8_t partnum = this->spi_read_status_(0x30);
  uint8_t version = this->spi_read_status_(0x31);
  ESP_LOGVV(TAG, "PARTNUM: 0x%02X, VERSION: 0x%02X", partnum, version);

  auto modem = build_modem_config(this->bitrate_bps_,
                                  static_cast<uint32_t>(this->deviation_hz_),
                                  static_cast<uint32_t>(this->rx_bandwidth_hz_),
                                  static_cast<uint32_t>(this->channel_spacing_hz_));
  uint32_t freq_word =
      compute_frequency_word(static_cast<uint32_t>(this->frequency_hz_));

  this->spi_write_reg_(REG_IOCFG0, 0x06); // IOCFG0: sync word detected
  this->spi_write_reg_(REG_FIFOTHR, 0x0F); // FIFOTHR: high threshold

  this->spi_write_reg_(REG_FSCTRL1, 0x06); // FSCTRL1: IF frequency
  this->spi_write_reg_(REG_FSCTRL0, 0x00); // FSCTRL0

  this->spi_write_burst_(REG_FREQ2,
                         {BYTE(freq_word, 2), BYTE(freq_word, 1), BYTE(freq_word, 0)}); // FREQ2/1/0

  this->spi_write_reg_(REG_MDMCFG4, modem.mdmcfg4);
  this->spi_write_reg_(REG_MDMCFG3, modem.mdmcfg3);
  this->spi_write_reg_(REG_MDMCFG2, modem.mdmcfg2);
  this->spi_write_reg_(REG_MDMCFG1, modem.mdmcfg1);
  this->spi_write_reg_(REG_MDMCFG0, modem.mdmcfg0);
  this->spi_write_reg_(REG_DEVIATN, modem.deviatn);

  this->spi_write_reg_(REG_SYNC1, 0x54);
  this->spi_write_reg_(REG_SYNC0, 0x3D);

  this->spi_write_reg_(REG_PKTLEN, 0xFF); // PKTLEN
  this->spi_write_reg_(REG_PKTCTRL1, 0x00); // PKTCTRL1
  this->spi_write_reg_(REG_PKTCTRL0, 0x02); // PKTCTRL0: infinite length, no CRC

  this->strobe_(0x34); // SRX
  delay(5);
  ESP_LOGV(TAG, "CC1101 setup done");
}

optional<uint8_t> CC1101::read() {
  if (this->irq_pin_->digital_read() == false)
    this->sync_seen_ = true;

  if (!this->sync_seen_)
    return {};

  uint8_t rxbytes = this->spi_read_status_(REG_RXBYTES);
  if (rxbytes & 0x80) {
    ESP_LOGW(TAG, "RX FIFO overflow");
    this->strobe_(0x3A); // SFRX
    this->strobe_(0x34); // SRX
    this->sync_seen_ = false;
    return {};
  }
  if ((rxbytes & 0x7F) == 0) {
    this->sync_seen_ = false;
    return {};
  }
  return this->spi_read_reg_(0x3F);
}

void CC1101::restart_rx() {
  this->strobe_(0x36); // SIDLE
  delay(1);
  this->strobe_(0x3A); // SFRX
  delay(1);
  this->strobe_(0x34); // SRX
  delay(1);
}

int8_t CC1101::get_rssi() {
  uint8_t raw = this->spi_read_status_(REG_RSSI);
  int16_t rssi_dec = raw >= 128 ? (int16_t)raw - 256 : raw;
  return (int8_t)(rssi_dec / 2 - 74);
}

const char *CC1101::get_name() { return TAG; }

uint8_t CC1101::spi_read_reg_(uint8_t address) {
  this->delegate_->begin_transaction();
  this->delegate_->transfer(0x80 | address);
  uint8_t value = this->delegate_->transfer(0x00);
  this->delegate_->end_transaction();
  return value;
}

uint8_t CC1101::spi_read_status_(uint8_t address) {
  this->delegate_->begin_transaction();
  this->delegate_->transfer(0xC0 | address);
  uint8_t value = this->delegate_->transfer(0x00);
  this->delegate_->end_transaction();
  return value;
}

void CC1101::spi_write_reg_(uint8_t address, uint8_t data) {
  this->delegate_->begin_transaction();
  this->delegate_->transfer(address);
  this->delegate_->transfer(data);
  this->delegate_->end_transaction();
}

void CC1101::spi_write_burst_(uint8_t address, std::initializer_list<uint8_t> data) {
  this->delegate_->begin_transaction();
  this->delegate_->transfer(0x40 | address);
  for (auto byte : data)
    this->delegate_->transfer(byte);
  this->delegate_->end_transaction();
}

void CC1101::strobe_(uint8_t command) {
  this->delegate_->begin_transaction();
  this->delegate_->transfer(command);
  this->delegate_->end_transaction();
}

void CC1101::set_frequency(float frequency_hz) {
  this->frequency_hz_ = frequency_hz;
}

void CC1101::set_bitrate(uint32_t bitrate_bps) {
  this->bitrate_bps_ = bitrate_bps;
}

void CC1101::set_deviation(float deviation_hz) {
  this->deviation_hz_ = deviation_hz;
}

void CC1101::set_rx_bandwidth(float bandwidth_hz) {
  this->rx_bandwidth_hz_ = bandwidth_hz;
}

void CC1101::set_channel_spacing(float spacing_hz) {
  this->channel_spacing_hz_ = spacing_hz;
}

} // namespace wmbus_radio
} // namespace esphome
