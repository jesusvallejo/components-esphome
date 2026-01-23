#include "transceiver_cc1101.h"
#include "esphome/core/log.h"
#include <cmath>

namespace esphome {
namespace wmbus_radio {

static const char *const TAG = "CC1101";

namespace {
// Helper function to split float into exponent and mantissa (like ESPHome)
void split_float(float value, int mbits, uint8_t &e, uint32_t &m) {
  int e_tmp;
  float m_tmp = std::frexp(value, &e_tmp);
  if (e_tmp <= mbits) {
    e = 0;
    m = 0;
    return;
  }
  e = static_cast<uint8_t>(e_tmp - mbits - 1);
  m = static_cast<uint32_t>(((m_tmp * 2 - 1) * (1 << (mbits + 1))));
}

// Calculate frequency register value
uint32_t compute_frequency_word(float frequency) {
  return static_cast<uint32_t>((frequency / XTAL_FREQUENCY) * (1 << 16));
}

// Calculate data rate registers
void compute_drate(uint32_t bitrate, uint8_t &drate_e, uint8_t &drate_m) {
  float value = (static_cast<float>(bitrate) * (1 << 28)) / XTAL_FREQUENCY;
  uint8_t e;
  uint32_t m;
  split_float(value, 8, e, m);
  drate_e = e;
  drate_m = static_cast<uint8_t>(m);
}

// Calculate deviation register
void compute_deviation(float deviation, uint8_t &dev_e, uint8_t &dev_m) {
  float value = (deviation * (1 << 17)) / XTAL_FREQUENCY;
  uint8_t e;
  uint32_t m;
  split_float(value, 3, e, m);
  dev_e = e;
  dev_m = static_cast<uint8_t>(m);
}

// Calculate RX filter bandwidth
void compute_rx_bw(float bandwidth, uint8_t &bw_e, uint8_t &bw_m) {
  // BW = XTAL / (8 * (4 + BW_M) * 2^BW_E)
  // Find best match
  uint32_t best_err = 0xFFFFFFFF;
  for (uint8_t e = 0; e <= 3; e++) {
    for (uint8_t m = 0; m <= 3; m++) {
      float bw = XTAL_FREQUENCY / (8.0f * (4 + m) * (1 << e));
      uint32_t err = static_cast<uint32_t>(std::abs(bw - bandwidth));
      if (err < best_err) {
        best_err = err;
        bw_e = e;
        bw_m = m;
      }
    }
  }
}

// Calculate channel spacing
void compute_channel_spacing(float spacing, uint8_t &cs_e, uint8_t &cs_m) {
  float value = (spacing * (1 << 18)) / XTAL_FREQUENCY;
  uint8_t e;
  uint32_t m;
  split_float(value, 8, e, m);
  cs_e = e;
  cs_m = static_cast<uint8_t>(m);
}

}  // namespace

void CC1101::setup() {
  this->common_setup();
  ESP_LOGI(TAG, "Setting up CC1101...");
  
  // Reset the chip
  this->reset();
  this->strobe_(CC1101Command::SRES);
  delay(10);

  // Read chip identification
  uint8_t partnum = this->read_status_(CC1101Register::PARTNUM);
  uint8_t version = this->read_status_(CC1101Register::VERSION);
  ESP_LOGI(TAG, "CC1101 PARTNUM: 0x%02X, VERSION: 0x%02X", partnum, version);

  // Calculate modem configuration
  uint8_t drate_e, drate_m, dev_e, dev_m, bw_e, bw_m, cs_e, cs_m;
  compute_drate(this->bitrate_bps_, drate_e, drate_m);
  compute_deviation(this->deviation_hz_, dev_e, dev_m);
  compute_rx_bw(this->rx_bandwidth_hz_, bw_e, bw_m);
  compute_channel_spacing(this->channel_spacing_hz_, cs_e, cs_m);

  uint32_t freq_word = compute_frequency_word(this->frequency_hz_);

  // Configure GDO pins
  // GDO0: 0x01 = Assert when RX FIFO >= threshold OR end of packet
  // This allows interrupt when data is available to read
  // Inverted (0x41) so it goes LOW when FIFO has data (for FALLING_EDGE interrupt)
  this->write_reg_(CC1101Register::IOCFG0, 0x41);
  // GDO2: CHIP_RDYn (not used)
  this->write_reg_(CC1101Register::IOCFG2, 0x29);

  // FIFO threshold: 0x04 = 17 bytes in RX FIFO triggers threshold
  // Lower threshold means earlier interrupt, less chance of overflow
  // At 100kbps, 17 bytes = ~1.4ms of data, leaves 47 bytes headroom
  this->write_reg_(CC1101Register::FIFOTHR, 0x04);

  // Frequency synthesizer control
  this->write_reg_(CC1101Register::FSCTRL1, 0x06);  // IF frequency ~152kHz
  this->write_reg_(CC1101Register::FSCTRL0, 0x00);

  // Frequency
  this->write_reg_(CC1101Register::FREQ2, (freq_word >> 16) & 0xFF);
  this->write_reg_(CC1101Register::FREQ1, (freq_word >> 8) & 0xFF);
  this->write_reg_(CC1101Register::FREQ0, freq_word & 0xFF);

  // Modem configuration
  uint8_t mdmcfg4 = (bw_e << 6) | (bw_m << 4) | (drate_e & 0x0F);
  uint8_t mdmcfg3 = drate_m;
  uint8_t mdmcfg2 = 0x02 | (1 << 4);  // 2-FSK, 16/16 sync word, sync mode = 16-bit
  uint8_t mdmcfg1 = (2 << 4) | (cs_e & 0x03);  // 4 preamble bytes, channel spacing exp
  uint8_t mdmcfg0 = cs_m;
  uint8_t deviatn = (dev_e << 4) | (dev_m & 0x07);

  this->write_reg_(CC1101Register::MDMCFG4, mdmcfg4);
  this->write_reg_(CC1101Register::MDMCFG3, mdmcfg3);
  this->write_reg_(CC1101Register::MDMCFG2, mdmcfg2);
  this->write_reg_(CC1101Register::MDMCFG1, mdmcfg1);
  this->write_reg_(CC1101Register::MDMCFG0, mdmcfg0);
  this->write_reg_(CC1101Register::DEVIATN, deviatn);

  // Sync word (WMBus Mode T: 0x543D)
  this->write_reg_(CC1101Register::SYNC1, 0x54);
  this->write_reg_(CC1101Register::SYNC0, 0x3D);

  // Main Radio Control State Machine
  this->write_reg_(CC1101Register::MCSM2, 0x07);  // RX_TIME disabled
  this->write_reg_(CC1101Register::MCSM1, 0x30);  // CCA mode: always, RX->RX after packet
  this->write_reg_(CC1101Register::MCSM0, 0x18);  // FS auto-cal when going from IDLE to RX/TX

  // Frequency offset compensation
  this->write_reg_(CC1101Register::FOCCFG, 0x16);

  // Bit synchronization
  this->write_reg_(CC1101Register::BSCFG, 0x6C);

  // AGC control
  this->write_reg_(CC1101Register::AGCCTRL2, 0x43);
  this->write_reg_(CC1101Register::AGCCTRL1, 0x40);
  this->write_reg_(CC1101Register::AGCCTRL0, 0x91);

  // Front end configuration
  this->write_reg_(CC1101Register::FREND1, 0x56);
  this->write_reg_(CC1101Register::FREND0, 0x10);

  // Frequency synthesizer calibration
  this->write_reg_(CC1101Register::FSCAL3, 0xE9);
  this->write_reg_(CC1101Register::FSCAL2, 0x2A);
  this->write_reg_(CC1101Register::FSCAL1, 0x00);
  this->write_reg_(CC1101Register::FSCAL0, 0x1F);

  // Test registers
  this->write_reg_(CC1101Register::TEST2, 0x81);
  this->write_reg_(CC1101Register::TEST1, 0x35);
  this->write_reg_(CC1101Register::TEST0, 0x09);

  // Packet configuration
  this->write_reg_(CC1101Register::PKTLEN, 0xFF);      // Max packet length
  this->write_reg_(CC1101Register::PKTCTRL1, 0x00);   // No address check, no auto flush
  this->write_reg_(CC1101Register::PKTCTRL0, 0x02);   // Infinite packet length mode, no CRC

  // Channel
  this->write_reg_(CC1101Register::CHANNR, 0x00);

  // Calibrate and enter RX
  this->strobe_(CC1101Command::SCAL);
  delay(1);
  
  if (!this->enter_rx_()) {
    ESP_LOGE(TAG, "Failed to enter RX mode!");
    this->mark_failed();
    return;
  }

  ESP_LOGI(TAG, "CC1101 setup complete - Freq: %.3f MHz, Bitrate: %u bps",
           this->frequency_hz_ / 1000000.0f, this->bitrate_bps_);
}

optional<uint8_t> CC1101::read() {
  // Return cached data first - this is the fast path
  if (this->fifo_cache_index_ < this->fifo_cache_size_) {
    return this->fifo_cache_[this->fifo_cache_index_++];
  }

  // Cache exhausted, need to read from FIFO
  this->fifo_cache_index_ = 0;
  this->fifo_cache_size_ = 0;

  // Read RXBYTES - check overflow FIRST (bit 7)
  uint8_t rxbytes = this->read_status_(CC1101Register::RXBYTES);
  
  if (rxbytes & 0x80) {
    // Overflow detected - must flush
    ESP_LOGW(TAG, "RX FIFO overflow");
    this->enter_idle_();
    this->strobe_(CC1101Command::SFRX);
    this->enter_rx_();
    this->sync_seen_ = false;
    return {};
  }

  uint8_t available = rxbytes & 0x7F;
  
  // If no data available, return empty
  if (available == 0) {
    return {};
  }

  // Read all available data immediately to prevent overflow
  // Don't read the last byte if FIFO is not empty to avoid SPI timing issues
  // (CC1101 errata: reading last byte while receiving can cause issues)
  size_t to_read = available;
  if (to_read > 1 && this->get_state_() == CC1101State::RX) {
    // While actively receiving, leave 1 byte in FIFO as safety margin
    to_read = available - 1;
  }
  
  if (to_read == 0) {
    return {};
  }

  // Burst read into cache
  this->fifo_cache_size_ = std::min<size_t>(to_read, this->fifo_cache_.size());
  this->read_burst_(CC1101Register::FIFO, this->fifo_cache_.data(), this->fifo_cache_size_);
  
  return this->fifo_cache_[this->fifo_cache_index_++];
}

void CC1101::restart_rx() {
  // Just ensure we're in RX mode and reset our state
  // Don't flush FIFO unless there's an actual problem
  CC1101State state = this->get_state_();
  
  if (state == CC1101State::RXFIFO_OVERFLOW) {
    // Need to flush on overflow
    this->enter_idle_();
    this->strobe_(CC1101Command::SFRX);
    delayMicroseconds(100);
  } else if (state != CC1101State::RX) {
    // Not in RX, need to enter RX
    this->enter_idle_();
    delayMicroseconds(100);
  }
  
  if (this->get_state_() != CC1101State::RX) {
    this->enter_rx_();
  }
  
  // Reset cache state
  this->fifo_cache_index_ = 0;
  this->fifo_cache_size_ = 0;
  this->sync_seen_ = false;
}

void CC1101::clear_rx() {
  // Force flush everything
  this->enter_idle_();
  this->strobe_(CC1101Command::SFRX);
  delayMicroseconds(100);
  this->enter_rx_();
  this->fifo_cache_index_ = 0;
  this->fifo_cache_size_ = 0;
  this->sync_seen_ = false;
}

void CC1101::flush_rx_() {
  this->enter_idle_();
  this->strobe_(CC1101Command::SFRX);
  delayMicroseconds(100);
  this->enter_rx_();
  this->fifo_cache_index_ = 0;
  this->fifo_cache_size_ = 0;
  this->sync_seen_ = false;
}

void CC1101::enter_idle_() {
  this->strobe_(CC1101Command::SIDLE);
  // Wait for IDLE state
  this->wait_for_state_(CC1101State::IDLE, 100);
}

bool CC1101::enter_rx_() {
  this->strobe_(CC1101Command::SRX);
  return this->wait_for_state_(CC1101State::RX, 100);
}

bool CC1101::wait_for_state_(CC1101State target, uint32_t timeout_ms) {
  uint32_t start = millis();
  while (millis() - start < timeout_ms) {
    CC1101State current = this->get_state_();
    if (current == target) {
      return true;
    }
    delayMicroseconds(100);
  }
  ESP_LOGW(TAG, "Timeout waiting for state %d, current: %d",
           static_cast<int>(target), static_cast<int>(this->get_state_()));
  return false;
}

CC1101State CC1101::get_state_() {
  uint8_t marcstate = this->read_status_(CC1101Register::MARCSTATE) & 0x1F;
  return static_cast<CC1101State>(marcstate);
}

int8_t CC1101::get_rssi() {
  uint8_t raw = this->read_status_(CC1101Register::RSSI);
  // Convert to signed and apply formula
  int16_t rssi = (raw >= 128) ? (static_cast<int16_t>(raw) - 256) : raw;
  return static_cast<int8_t>((rssi * RSSI_STEP) - RSSI_OFFSET);
}

const char *CC1101::get_name() { return TAG; }

// SPI Operations using ESPHome-style bus flags

uint8_t CC1101::strobe_(CC1101Command cmd) {
  uint8_t index = static_cast<uint8_t>(cmd);
  this->delegate_->begin_transaction();
  uint8_t status = this->delegate_->transfer(index);
  this->delegate_->end_transaction();
  return status;
}

void CC1101::write_reg_(CC1101Register reg, uint8_t value) {
  uint8_t index = static_cast<uint8_t>(reg);
  this->delegate_->begin_transaction();
  this->delegate_->transfer(index | BUS_WRITE);
  this->delegate_->transfer(value);
  this->delegate_->end_transaction();
}

void CC1101::write_burst_(CC1101Register reg, const uint8_t *data, size_t length) {
  uint8_t index = static_cast<uint8_t>(reg);
  this->delegate_->begin_transaction();
  this->delegate_->transfer(index | BUS_WRITE | BUS_BURST);
  for (size_t i = 0; i < length; i++) {
    this->delegate_->transfer(data[i]);
  }
  this->delegate_->end_transaction();
}

uint8_t CC1101::read_reg_(CC1101Register reg) {
  uint8_t index = static_cast<uint8_t>(reg);
  this->delegate_->begin_transaction();
  this->delegate_->transfer(index | BUS_READ);
  uint8_t value = this->delegate_->transfer(0x00);
  this->delegate_->end_transaction();
  return value;
}

void CC1101::read_burst_(CC1101Register reg, uint8_t *data, size_t length) {
  uint8_t index = static_cast<uint8_t>(reg);
  this->delegate_->begin_transaction();
  this->delegate_->transfer(index | BUS_READ | BUS_BURST);
  for (size_t i = 0; i < length; i++) {
    data[i] = this->delegate_->transfer(0x00);
  }
  this->delegate_->end_transaction();
}

uint8_t CC1101::read_status_(CC1101Register reg) {
  // Status registers (0x30-0x3D) need burst bit set to read
  uint8_t index = static_cast<uint8_t>(reg);
  this->delegate_->begin_transaction();
  this->delegate_->transfer(index | BUS_READ | BUS_BURST);
  uint8_t value = this->delegate_->transfer(0x00);
  this->delegate_->end_transaction();
  return value;
}

// Configuration setters

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

}  // namespace wmbus_radio
}  // namespace esphome
