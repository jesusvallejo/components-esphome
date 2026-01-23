#pragma once

#include "esphome/core/hal.h"

#include <cstdint>

namespace esphome {
namespace wmbus {

// CC1101 Register addresses
#define CC1101_IOCFG2       0x00
#define CC1101_IOCFG1       0x01
#define CC1101_IOCFG0       0x02
#define CC1101_FIFOTHR      0x03
#define CC1101_SYNC1        0x04
#define CC1101_SYNC0        0x05
#define CC1101_PKTLEN       0x06
#define CC1101_PKTCTRL1     0x07
#define CC1101_PKTCTRL0     0x08
#define CC1101_ADDR         0x09
#define CC1101_CHANNR       0x0A
#define CC1101_FSCTRL1      0x0B
#define CC1101_FSCTRL0      0x0C
#define CC1101_FREQ2        0x0D
#define CC1101_FREQ1        0x0E
#define CC1101_FREQ0        0x0F
#define CC1101_MDMCFG4      0x10
#define CC1101_MDMCFG3      0x11
#define CC1101_MDMCFG2      0x12
#define CC1101_MDMCFG1      0x13
#define CC1101_MDMCFG0      0x14
#define CC1101_DEVIATN      0x15
#define CC1101_MCSM2        0x16
#define CC1101_MCSM1        0x17
#define CC1101_MCSM0        0x18
#define CC1101_FOCCFG       0x19
#define CC1101_BSCFG        0x1A
#define CC1101_AGCCTRL2     0x1B
#define CC1101_AGCCTRL1     0x1C
#define CC1101_AGCCTRL0     0x1D
#define CC1101_WOREVT1      0x1E
#define CC1101_WOREVT0      0x1F
#define CC1101_WORCTRL      0x20
#define CC1101_FREND1       0x21
#define CC1101_FREND0       0x22
#define CC1101_FSCAL3       0x23
#define CC1101_FSCAL2       0x24
#define CC1101_FSCAL1       0x25
#define CC1101_FSCAL0       0x26
#define CC1101_RCCTRL1      0x27
#define CC1101_RCCTRL0      0x28
#define CC1101_FSTEST       0x29
#define CC1101_PTEST        0x2A
#define CC1101_AGCTEST      0x2B
#define CC1101_TEST2        0x2C
#define CC1101_TEST1        0x2D
#define CC1101_TEST0        0x2E

// CC1101 Status registers
#define CC1101_PARTNUM      0x30
#define CC1101_VERSION      0x31
#define CC1101_FREQEST      0x32
#define CC1101_LQI          0x33
#define CC1101_RSSI         0x34
#define CC1101_MARCSTATE    0x35
#define CC1101_WORTIME1     0x36
#define CC1101_WORTIME0     0x37
#define CC1101_PKTSTATUS    0x38
#define CC1101_VCO_VC_DAC   0x39
#define CC1101_TXBYTES      0x3A
#define CC1101_RXBYTES      0x3B
#define CC1101_RCCTRL1_STATUS 0x3C
#define CC1101_RCCTRL0_STATUS 0x3D

// CC1101 Strobe commands
#define CC1101_SRES         0x30
#define CC1101_SFSTXON      0x31
#define CC1101_SXOFF        0x32
#define CC1101_SCAL         0x33
#define CC1101_SRX          0x34
#define CC1101_STX          0x35
#define CC1101_SIDLE        0x36
#define CC1101_SWOR         0x38
#define CC1101_SPWD         0x39
#define CC1101_SFRX         0x3A
#define CC1101_SFTX         0x3B
#define CC1101_SWORRST      0x3C
#define CC1101_SNOP         0x3D

// CC1101 FIFO addresses
#define CC1101_TXFIFO       0x3F
#define CC1101_RXFIFO       0x3F

// CC1101 PATABLE address
#define CC1101_PATABLE      0x3E

// SPI access modes
#define CC1101_WRITE_SINGLE 0x00
#define CC1101_WRITE_BURST  0x40
#define CC1101_READ_SINGLE  0x80
#define CC1101_READ_BURST   0xC0

/**
 * @brief Native CC1101 driver for ESP-IDF using ESPHome's SPI abstraction
 * 
 * This class provides a native implementation of the CC1101 radio driver
 * that does not depend on the Arduino framework or external libraries.
 */
class CC1101 {
 public:
  CC1101() = default;

  /**
   * @brief Initialize the SPI interface for CC1101
   * 
   * @param mosi MOSI pin number
   * @param miso MISO pin number
   * @param clk Clock pin number
   * @param cs Chip select pin number
   * @return true if SPI initialization successful
   */
  bool init_spi(uint8_t mosi, uint8_t miso, uint8_t clk, uint8_t cs);

  /**
   * @brief Initialize the CC1101 chip
   * 
   * @return true if CC1101 initialization successful
   */
  bool init();

  /**
   * @brief Reset the CC1101 chip
   */
  void reset();

  /**
   * @brief Write a single byte to a CC1101 register
   * 
   * @param addr Register address
   * @param value Value to write
   */
  void write_reg(uint8_t addr, uint8_t value);

  /**
   * @brief Write multiple bytes to CC1101 registers (burst mode)
   * 
   * @param addr Starting register address
   * @param buffer Pointer to data buffer
   * @param length Number of bytes to write
   */
  void write_burst(uint8_t addr, const uint8_t *buffer, uint8_t length);

  /**
   * @brief Read a single byte from a CC1101 register
   * 
   * @param addr Register address
   * @return Register value
   */
  uint8_t read_reg(uint8_t addr);

  /**
   * @brief Read a CC1101 status register
   * 
   * @param addr Status register address
   * @return Status register value
   */
  uint8_t read_status(uint8_t addr);

  /**
   * @brief Read multiple bytes from CC1101 registers (burst mode)
   * 
   * @param addr Starting register address
   * @param buffer Pointer to buffer to store data
   * @param length Number of bytes to read
   */
  void read_burst(uint8_t addr, uint8_t *buffer, uint8_t length);

  /**
   * @brief Send a strobe command to CC1101
   * 
   * @param strobe Strobe command
   * @return Status byte
   */
  uint8_t strobe(uint8_t strobe);

  /**
   * @brief Set CC1101 to RX mode
   */
  void set_rx();

  /**
   * @brief Set CC1101 to idle mode
   */
  void set_idle();

  /**
   * @brief Get RSSI value in dBm
   * 
   * @return RSSI value in dBm
   */
  int8_t get_rssi();

  /**
   * @brief Get LQI value
   * 
   * @return LQI value
   */
  uint8_t get_lqi();

  /**
   * @brief Get CC1101 version
   * 
   * @return Version number
   */
  uint8_t get_version();

 protected:
  void spi_begin_();
  void spi_end_();
  uint8_t spi_transfer_(uint8_t data);
  void wait_miso_low_();

  uint8_t mosi_pin_{0};
  uint8_t miso_pin_{0};
  uint8_t clk_pin_{0};
  uint8_t cs_pin_{0};

#ifdef USE_ESP_IDF
  void *spi_handle_{nullptr};
#endif
  bool spi_initialized_{false};
};

// Global instance
extern CC1101 cc1101;

}  // namespace wmbus
}  // namespace esphome
