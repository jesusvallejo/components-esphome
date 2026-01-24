#pragma once

#include "esphome/core/log.h"

#include "mbus.h"
#include "utils_my.h"
#include "decode3of6.h"
#include "m_bus_data.h"
#include "cc1101_rf_settings.h"
#include "cc1101.h"

#include <cstdint>
#include <string>

namespace esphome {
namespace wmbus {

// ============================================================================
// CC1101 MARCSTATE Values
// ============================================================================

enum class MarcState : uint8_t {
  SLEEP            = 0x00,
  IDLE             = 0x01,
  XOFF             = 0x02,
  VCOON_MC         = 0x03,
  REGON_MC         = 0x04,
  MANCAL           = 0x05,
  VCOON            = 0x06,
  REGON            = 0x07,
  STARTCAL         = 0x08,
  BWBOOST          = 0x09,
  FS_LOCK          = 0x0A,
  IFADCON          = 0x0B,
  ENDCAL           = 0x0C,
  RX               = 0x0D,
  RX_END           = 0x0E,
  RX_RST           = 0x0F,
  TXRX_SWITCH      = 0x10,
  RXFIFO_OVERFLOW  = 0x11,
  FSTXON           = 0x12,
  TX               = 0x13,
  TX_END           = 0x14,
  RXTX_SWITCH      = 0x15,
  TXFIFO_UNDERFLOW = 0x16,
};

// ============================================================================
// RX Configuration Constants
// ============================================================================

static constexpr uint8_t RX_FIFO_START_THRESHOLD = 0;   // Initial threshold: 4 bytes
static constexpr uint8_t RX_FIFO_THRESHOLD = 10;        // 44 bytes in Rx FIFO

static constexpr uint8_t FIXED_PACKET_LENGTH = 0x00;
static constexpr uint8_t INFINITE_PACKET_LENGTH = 0x02;

static constexpr uint16_t MAX_FIXED_LENGTH = 256;

// wM-Bus preamble bytes
static constexpr uint8_t WMBUS_MODE_C_PREAMBLE = 0x54;
static constexpr uint8_t WMBUS_BLOCK_A_PREAMBLE = 0xCD;
static constexpr uint8_t WMBUS_BLOCK_B_PREAMBLE = 0x3D;

// ============================================================================
// RX Loop State Machine
// ============================================================================

enum class RxState : uint8_t {
  INIT          = 0,
  WAIT_FOR_SYNC = 1,
  WAIT_FOR_DATA = 2,
  READ_DATA     = 3,
};

enum class PacketLengthMode : uint8_t {
  INFINITE = 0,
  FIXED    = 1,
};

// ============================================================================
// RX Loop Data Structure
// ============================================================================

struct RxLoopData {
  uint16_t bytes_received{0};
  uint8_t length_field{0};        // The L-field in the wM-Bus packet
  uint16_t total_length{0};       // Total number of bytes to read from RX FIFO
  uint16_t bytes_remaining{0};    // Bytes left to read from RX FIFO
  uint8_t *buffer_ptr{nullptr};   // Pointer to current position in byte array
  bool complete{false};           // Packet received complete
  PacketLengthMode length_mode{PacketLengthMode::INFINITE};
  RxState state{RxState::INIT};
};

// ============================================================================
// RxLoop Class - wM-Bus Reception Handler
// ============================================================================

class RxLoop {
 public:
  /**
   * @brief Initialize the CC1101 for wM-Bus reception
   * @param mosi MOSI pin
   * @param miso MISO pin
   * @param clk Clock pin
   * @param cs Chip select pin
   * @param gdo0 GDO0 pin (RX FIFO threshold)
   * @param gdo2 GDO2 pin (sync word detected)
   * @param freq Frequency in MHz
   * @param sync_mode Enable synchronous mode
   * @return true if initialization successful
   */
  bool init(uint8_t mosi, uint8_t miso, uint8_t clk, uint8_t cs,
            uint8_t gdo0, uint8_t gdo2, float freq, bool sync_mode);

  /**
   * @brief Process RX state machine
   * @return true if a complete frame was received
   */
  bool task();

  /**
   * @brief Get the last received frame
   * @return The received wM-Bus frame
   */
  WMbusFrame get_frame();

 private:
  bool start(bool force = true);
  uint8_t get_marc_state();

  // Configuration
  bool sync_mode_{false};
  uint8_t gdo0_pin_{0};
  uint8_t gdo2_pin_{0};

  // Data buffers
  WMbusData rx_data_{};           // Data from physical layer decoded to bytes
  WMbusFrame result_frame_{};

  // State machine
  RxLoopData rx_loop_{};

  // Timing
  uint32_t sync_time_{0};
  static constexpr uint8_t EXTRA_TIME_MS = 50;
  uint8_t max_wait_time_{EXTRA_TIME_MS};
};

}  // namespace wmbus
}  // namespace esphome