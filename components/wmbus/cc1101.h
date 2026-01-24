#pragma once

#include <cstdint>

namespace esphome {
namespace wmbus {

// ============================================================================
// CC1101 Register Addresses
// ============================================================================

// Configuration registers (0x00 - 0x2E)
static constexpr uint8_t CC1101_IOCFG2       = 0x00;
static constexpr uint8_t CC1101_IOCFG1       = 0x01;
static constexpr uint8_t CC1101_IOCFG0       = 0x02;
static constexpr uint8_t CC1101_FIFOTHR      = 0x03;
static constexpr uint8_t CC1101_SYNC1        = 0x04;
static constexpr uint8_t CC1101_SYNC0        = 0x05;
static constexpr uint8_t CC1101_PKTLEN       = 0x06;
static constexpr uint8_t CC1101_PKTCTRL1     = 0x07;
static constexpr uint8_t CC1101_PKTCTRL0     = 0x08;
static constexpr uint8_t CC1101_ADDR         = 0x09;
static constexpr uint8_t CC1101_CHANNR       = 0x0A;
static constexpr uint8_t CC1101_FSCTRL1      = 0x0B;
static constexpr uint8_t CC1101_FSCTRL0      = 0x0C;
static constexpr uint8_t CC1101_FREQ2        = 0x0D;
static constexpr uint8_t CC1101_FREQ1        = 0x0E;
static constexpr uint8_t CC1101_FREQ0        = 0x0F;
static constexpr uint8_t CC1101_MDMCFG4      = 0x10;
static constexpr uint8_t CC1101_MDMCFG3      = 0x11;
static constexpr uint8_t CC1101_MDMCFG2      = 0x12;
static constexpr uint8_t CC1101_MDMCFG1      = 0x13;
static constexpr uint8_t CC1101_MDMCFG0      = 0x14;
static constexpr uint8_t CC1101_DEVIATN      = 0x15;
static constexpr uint8_t CC1101_MCSM2        = 0x16;
static constexpr uint8_t CC1101_MCSM1        = 0x17;
static constexpr uint8_t CC1101_MCSM0        = 0x18;
static constexpr uint8_t CC1101_FOCCFG       = 0x19;
static constexpr uint8_t CC1101_BSCFG        = 0x1A;
static constexpr uint8_t CC1101_AGCCTRL2     = 0x1B;
static constexpr uint8_t CC1101_AGCCTRL1     = 0x1C;
static constexpr uint8_t CC1101_AGCCTRL0     = 0x1D;
static constexpr uint8_t CC1101_WOREVT1      = 0x1E;
static constexpr uint8_t CC1101_WOREVT0      = 0x1F;
static constexpr uint8_t CC1101_WORCTRL      = 0x20;
static constexpr uint8_t CC1101_FREND1       = 0x21;
static constexpr uint8_t CC1101_FREND0       = 0x22;
static constexpr uint8_t CC1101_FSCAL3       = 0x23;
static constexpr uint8_t CC1101_FSCAL2       = 0x24;
static constexpr uint8_t CC1101_FSCAL1       = 0x25;
static constexpr uint8_t CC1101_FSCAL0       = 0x26;
static constexpr uint8_t CC1101_RCCTRL1      = 0x27;
static constexpr uint8_t CC1101_RCCTRL0      = 0x28;
static constexpr uint8_t CC1101_FSTEST       = 0x29;
static constexpr uint8_t CC1101_PTEST        = 0x2A;
static constexpr uint8_t CC1101_AGCTEST      = 0x2B;
static constexpr uint8_t CC1101_TEST2        = 0x2C;
static constexpr uint8_t CC1101_TEST1        = 0x2D;
static constexpr uint8_t CC1101_TEST0        = 0x2E;

// Status registers (0x30 - 0x3D) - read with burst bit set
static constexpr uint8_t CC1101_PARTNUM          = 0x30;
static constexpr uint8_t CC1101_VERSION          = 0x31;
static constexpr uint8_t CC1101_FREQEST          = 0x32;
static constexpr uint8_t CC1101_LQI              = 0x33;
static constexpr uint8_t CC1101_RSSI             = 0x34;
static constexpr uint8_t CC1101_MARCSTATE        = 0x35;
static constexpr uint8_t CC1101_WORTIME1         = 0x36;
static constexpr uint8_t CC1101_WORTIME0         = 0x37;
static constexpr uint8_t CC1101_PKTSTATUS        = 0x38;
static constexpr uint8_t CC1101_VCO_VC_DAC       = 0x39;
static constexpr uint8_t CC1101_TXBYTES          = 0x3A;
static constexpr uint8_t CC1101_RXBYTES          = 0x3B;
static constexpr uint8_t CC1101_RCCTRL1_STATUS   = 0x3C;
static constexpr uint8_t CC1101_RCCTRL0_STATUS   = 0x3D;

// Strobe commands (directly written to chip)
static constexpr uint8_t CC1101_SRES         = 0x30;  // Reset chip
static constexpr uint8_t CC1101_SFSTXON      = 0x31;  // Enable and calibrate frequency synthesizer
static constexpr uint8_t CC1101_SXOFF        = 0x32;  // Turn off crystal oscillator
static constexpr uint8_t CC1101_SCAL         = 0x33;  // Calibrate frequency synthesizer
static constexpr uint8_t CC1101_SRX          = 0x34;  // Enable RX
static constexpr uint8_t CC1101_STX          = 0x35;  // Enable TX
static constexpr uint8_t CC1101_SIDLE        = 0x36;  // Exit RX/TX, turn off frequency synthesizer
static constexpr uint8_t CC1101_SWOR         = 0x38;  // Start automatic RX polling sequence
static constexpr uint8_t CC1101_SPWD         = 0x39;  // Enter power down mode
static constexpr uint8_t CC1101_SFRX         = 0x3A;  // Flush the RX FIFO buffer
static constexpr uint8_t CC1101_SFTX         = 0x3B;  // Flush the TX FIFO buffer
static constexpr uint8_t CC1101_SWORRST      = 0x3C;  // Reset real time clock
static constexpr uint8_t CC1101_SNOP         = 0x3D;  // No operation

// FIFO and PATABLE addresses
static constexpr uint8_t CC1101_TXFIFO       = 0x3F;
static constexpr uint8_t CC1101_RXFIFO       = 0x3F;
static constexpr uint8_t CC1101_PATABLE      = 0x3E;

// SPI access mode bits
static constexpr uint8_t CC1101_WRITE_SINGLE = 0x00;
static constexpr uint8_t CC1101_WRITE_BURST  = 0x40;
static constexpr uint8_t CC1101_READ_SINGLE  = 0x80;
static constexpr uint8_t CC1101_READ_BURST   = 0xC0;

// ============================================================================
// CC1101 Configuration
// ============================================================================

// RSSI offset for dBm calculation (chip-specific, typically 74)
static constexpr int8_t CC1101_RSSI_OFFSET = 74;

// SPI clock speed in Hz
static constexpr uint32_t CC1101_SPI_CLOCK_HZ = 1000000;  // 1 MHz

// ============================================================================
// CC1101 Driver Class
// ============================================================================

/**
 * @brief Native CC1101 driver for ESP-IDF
 * 
 * This class provides a native implementation of the CC1101 radio driver
 * using ESP-IDF's SPI master driver. ESP-IDF framework only.
 */
class CC1101 {
 public:
  CC1101() = default;
  ~CC1101() = default;

  // Prevent copying
  CC1101(const CC1101&) = delete;
  CC1101& operator=(const CC1101&) = delete;

  // ---- Initialization ----
  
  /**
   * @brief Initialize the SPI interface for CC1101
   * @param mosi MOSI pin number
   * @param miso MISO pin number  
   * @param clk Clock pin number
   * @param cs Chip select pin number
   * @return true if SPI initialization successful
   */
  bool init_spi(uint8_t mosi, uint8_t miso, uint8_t clk, uint8_t cs);

  /**
   * @brief Initialize the CC1101 chip (reset and verify communication)
   * @return true if CC1101 initialization successful
   */
  bool init();

  /**
   * @brief Reset the CC1101 chip
   */
  void reset();

  /**
   * @brief Check if SPI is initialized
   * @return true if SPI is ready
   */
  bool is_initialized() const { return spi_initialized_; }

  // ---- Register Access ----

  void write_reg(uint8_t addr, uint8_t value);
  void write_burst(uint8_t addr, const uint8_t *buffer, uint8_t length);
  uint8_t read_reg(uint8_t addr);
  uint8_t read_status(uint8_t addr);
  void read_burst(uint8_t addr, uint8_t *buffer, uint8_t length);

  // ---- Command Strobes ----

  uint8_t strobe(uint8_t cmd);
  void set_rx();
  void set_idle();

  // ---- Status ----

  int8_t get_rssi();
  uint8_t get_lqi();
  uint8_t get_version();

 private:
  void spi_begin_();
  void spi_end_();
  uint8_t spi_transfer_(uint8_t data);
  void wait_miso_low_();

  // Pin configuration
  uint8_t mosi_pin_{0};
  uint8_t miso_pin_{0};
  uint8_t clk_pin_{0};
  uint8_t cs_pin_{0};

  // SPI state
  void *spi_handle_{nullptr};
  bool spi_initialized_{false};
};

// Global CC1101 instance
extern CC1101 cc1101;

}  // namespace wmbus
}  // namespace esphome
