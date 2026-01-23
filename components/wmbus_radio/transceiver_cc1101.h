#pragma once
#include "transceiver.h"
#include <array>

namespace esphome {
namespace wmbus_radio {

// CC1101 Register addresses
enum class CC1101Register : uint8_t {
  IOCFG2 = 0x00,
  IOCFG1 = 0x01,
  IOCFG0 = 0x02,
  FIFOTHR = 0x03,
  SYNC1 = 0x04,
  SYNC0 = 0x05,
  PKTLEN = 0x06,
  PKTCTRL1 = 0x07,
  PKTCTRL0 = 0x08,
  ADDR = 0x09,
  CHANNR = 0x0A,
  FSCTRL1 = 0x0B,
  FSCTRL0 = 0x0C,
  FREQ2 = 0x0D,
  FREQ1 = 0x0E,
  FREQ0 = 0x0F,
  MDMCFG4 = 0x10,
  MDMCFG3 = 0x11,
  MDMCFG2 = 0x12,
  MDMCFG1 = 0x13,
  MDMCFG0 = 0x14,
  DEVIATN = 0x15,
  MCSM2 = 0x16,
  MCSM1 = 0x17,
  MCSM0 = 0x18,
  FOCCFG = 0x19,
  BSCFG = 0x1A,
  AGCCTRL2 = 0x1B,
  AGCCTRL1 = 0x1C,
  AGCCTRL0 = 0x1D,
  WOREVT1 = 0x1E,
  WOREVT0 = 0x1F,
  WORCTRL = 0x20,
  FREND1 = 0x21,
  FREND0 = 0x22,
  FSCAL3 = 0x23,
  FSCAL2 = 0x24,
  FSCAL1 = 0x25,
  FSCAL0 = 0x26,
  RCCTRL1 = 0x27,
  RCCTRL0 = 0x28,
  FSTEST = 0x29,
  PTEST = 0x2A,
  AGCTEST = 0x2B,
  TEST2 = 0x2C,
  TEST1 = 0x2D,
  TEST0 = 0x2E,
  // Status registers (read with burst bit)
  PARTNUM = 0x30,
  VERSION = 0x31,
  FREQEST = 0x32,
  LQI = 0x33,
  RSSI = 0x34,
  MARCSTATE = 0x35,
  WORTIME1 = 0x36,
  WORTIME0 = 0x37,
  PKTSTATUS = 0x38,
  VCO_VC_DAC = 0x39,
  TXBYTES = 0x3A,
  RXBYTES = 0x3B,
  RCCTRL1_STATUS = 0x3C,
  RCCTRL0_STATUS = 0x3D,
  // FIFO
  FIFO = 0x3F,
};

// CC1101 Command strobes
enum class CC1101Command : uint8_t {
  SRES = 0x30,    // Reset chip
  SFSTXON = 0x31, // Enable and calibrate frequency synthesizer
  SXOFF = 0x32,   // Turn off crystal oscillator
  SCAL = 0x33,    // Calibrate frequency synthesizer and turn it off
  SRX = 0x34,     // Enable RX
  STX = 0x35,     // Enable TX
  SIDLE = 0x36,   // Exit RX/TX, turn off frequency synthesizer
  SAFC = 0x37,    // Perform AFC adjustment of the frequency synthesizer
  SWOR = 0x38,    // Start automatic RX polling sequence
  SPWD = 0x39,    // Enter power down mode when CSn goes high
  SFRX = 0x3A,    // Flush the RX FIFO buffer
  SFTX = 0x3B,    // Flush the TX FIFO buffer
  SWORRST = 0x3C, // Reset real time clock to Event1 value
  SNOP = 0x3D,    // No operation
};

// CC1101 MARCSTATE values
enum class CC1101State : uint8_t {
  SLEEP = 0x00,
  IDLE = 0x01,
  XOFF = 0x02,
  VCOON_MC = 0x03,
  REGON_MC = 0x04,
  MANCAL = 0x05,
  VCOON = 0x06,
  REGON = 0x07,
  STARTCAL = 0x08,
  BWBOOST = 0x09,
  FS_LOCK = 0x0A,
  IFADCON = 0x0B,
  ENDCAL = 0x0C,
  RX = 0x0D,
  RX_END = 0x0E,
  RX_RST = 0x0F,
  TXRX_SWITCH = 0x10,
  RXFIFO_OVERFLOW = 0x11,
  FSTXON = 0x12,
  TX = 0x13,
  TX_END = 0x14,
  RXTX_SWITCH = 0x15,
  TXFIFO_UNDERFLOW = 0x16,
};

// SPI bus flags (like ESPHome's cc1101defs.h)
static constexpr uint8_t BUS_WRITE = 0x00;
static constexpr uint8_t BUS_READ = 0x80;
static constexpr uint8_t BUS_BURST = 0x40;

// Crystal frequency and RSSI calculation
static constexpr float XTAL_FREQUENCY = 26000000.0f;
static constexpr float RSSI_OFFSET = 74.0f;
static constexpr float RSSI_STEP = 0.5f;

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
  // SPI operations (using ESPHome-style naming)
  uint8_t strobe_(CC1101Command cmd);
  void write_reg_(CC1101Register reg, uint8_t value);
  void write_burst_(CC1101Register reg, const uint8_t *data, size_t length);
  uint8_t read_reg_(CC1101Register reg);
  void read_burst_(CC1101Register reg, uint8_t *data, size_t length);
  uint8_t read_status_(CC1101Register reg);

  // State management (like ESPHome)
  void enter_idle_();
  bool enter_rx_();
  bool wait_for_state_(CC1101State target, uint32_t timeout_ms = 100);
  CC1101State get_state_();
  void flush_rx_();

  // Configuration
  float frequency_hz_{868950000.0f};
  uint32_t bitrate_bps_{100000};
  float deviation_hz_{50000.0f};
  float rx_bandwidth_hz_{200000.0f};
  float channel_spacing_hz_{200000.0f};

  // RX FIFO cache for efficient reading
  std::array<uint8_t, 64> fifo_cache_{};
  size_t fifo_cache_size_{0};
  size_t fifo_cache_index_{0};
  bool sync_seen_{false};
};

}  // namespace wmbus_radio
}  // namespace esphome
