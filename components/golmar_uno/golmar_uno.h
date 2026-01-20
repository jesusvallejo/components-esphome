#pragma once

#include <array>
#include <functional>

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/uart/uart.h"

#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif

#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif

#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif

#ifdef USE_LOCK
#include "esphome/components/lock/lock.h"
#endif

namespace esphome {
namespace golmar_uno {

/// Golmar UNO protocol payload size
static constexpr size_t PAYLOAD_SIZE = 4;

/// Static/fixed addresses and commands used by the Golmar UNO protocol
static constexpr uint8_t NO_ADDRESS = 0x00;
static constexpr uint8_t CLEAR_BUS_COMMAND = 0x11;
static constexpr uint8_t CONFIRM_COMMAND = 0x01;

static constexpr uint8_t INTERCOM_ADDRESS1 = NO_ADDRESS;
static constexpr uint8_t INTERCOM_ADDRESS2 = NO_ADDRESS;
static constexpr uint8_t INTERCOM_CALL_COMMAND = 0x37;

static constexpr uint8_t CONCIERGE_ADDRESS1 = NO_ADDRESS;
static constexpr uint8_t CONCIERGE_ADDRESS2 = NO_ADDRESS;
static constexpr uint8_t CONCIERGE_CALL_COMMAND = 0x22;
static constexpr uint8_t CONCIERGE_UNLOCK_COMMAND = 0x90;

/// Default duration for call alert in milliseconds
static constexpr uint32_t DEFAULT_CALL_ALERT_DURATION_MS = 2000;
/// Default delay between commands in milliseconds
static constexpr uint32_t DEFAULT_INTER_COMMAND_DELAY_MS = 500;
/// Timeout for confirmation in milliseconds
static constexpr uint32_t CONFIRMATION_TIMEOUT_MS = 5000;
/// Duration before clearing bus after unlock command
static constexpr uint32_t CLEAR_BUS_DELAY_MS = 1500;
/// Duration before auto-locking door
static constexpr uint32_t AUTO_LOCK_DELAY_MS = 2000;
/// Duration before auto-turning off switch
static constexpr uint32_t AUTO_SWITCH_OFF_DELAY_MS = 2000;

/**
 * @brief Main component class for Golmar UNO intercom system integration.
 *
 * This component communicates with Golmar UNO intercom systems via UART,
 * providing functionality to detect incoming calls and unlock doors.
 */
class GolmarUnoComponent : public Component, public uart::UARTDevice {
 public:
  /// @brief Initiate the door unlock sequence.
  void unlock();

  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_intercom_id(uint8_t intercom_id) { this->intercom_id_ = intercom_id; }
  void set_concierge_id(uint8_t concierge_id) { this->concierge_id_ = concierge_id; }

#ifdef USE_BINARY_SENSOR
  void set_calling_alert_binary_sensor(binary_sensor::BinarySensor *sensor) {
    this->calling_alert_binary_sensor_ = sensor;
  }
#endif

#ifdef USE_BUTTON
  void set_unlock_door_button(button::Button *button) { this->unlock_door_button_ = button; }
#endif

#ifdef USE_SWITCH
  void set_unlock_door_switch(switch_::Switch *switch_) { this->unlock_door_switch_ = switch_; }
  void schedule_switch_off(uint32_t delay_ms);
#endif

#ifdef USE_LOCK
  void set_door_lock(lock::Lock *lock) { this->door_lock_ = lock; }
  void schedule_door_lock(uint32_t delay_ms);
#endif

 protected:
  /// @brief Process a received byte and match against expected payload.
  void process_payload(uint8_t byte, const std::array<uint8_t, PAYLOAD_SIZE> &payload,
                       size_t &match_index, const char *message,
                       const std::function<void()> &on_match);

  /// @brief Write a 4-byte payload to the UART bus.
  void write_payload(uint8_t address1, uint8_t address2, uint8_t address3, uint8_t command);

  /// @brief Send a command to the concierge using the configured concierge_id.
  void write_concierge_command(uint8_t command);

  /// @brief Send a command to the intercom using the configured intercom_id.
  void write_intercom_command(uint8_t command);

  /// @brief Clear the communication bus.
  void clear_bus();

  /// @brief Handle incoming call detection.
  void handle_incoming_call(uint8_t byte);

  /// @brief Handle intercom confirmation messages.
  void handle_intercom_confirm(uint8_t byte);

  /// @brief Handle concierge confirmation messages.
  void handle_concierge_confirm(uint8_t byte);

  /// @brief Called when communication error occurs (no confirmation received).
  void on_communication_error();

#ifdef USE_BINARY_SENSOR
  binary_sensor::BinarySensor *calling_alert_binary_sensor_{nullptr};
#endif

#ifdef USE_BUTTON
  button::Button *unlock_door_button_{nullptr};
#endif

#ifdef USE_SWITCH
  switch_::Switch *unlock_door_switch_{nullptr};
#endif

#ifdef USE_LOCK
  lock::Lock *door_lock_{nullptr};
#endif

  uint8_t intercom_id_{0};
  uint8_t concierge_id_{0};

  /// State tracking for payload matching
  size_t incoming_match_index_{0};
  size_t concierge_confirm_match_index_{0};
  size_t intercom_confirm_match_index_{0};

  /// Callbacks for confirmation handling
  std::function<void()> on_confirm_concierge_{};
  std::function<void()> on_confirm_intercom_{};

  /// Flag indicating if unlock sequence is in progress
  bool unlock_sequence_active_{false};
};

}  // namespace golmar_uno
}  // namespace esphome