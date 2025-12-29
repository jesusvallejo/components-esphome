#pragma once

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

namespace esphome::golmar_uno {

// Static/fixed addresses and commands used by the Golmar UNO protocol
constexpr uint8_t INTERCOM_ADDRESS1 = 0x00;
constexpr uint8_t INTERCOM_ADDRESS2 = 0x00;
constexpr uint8_t INTERCOM_CALL_COMMAND = 0x37;
constexpr uint8_t NO_ADDRESS = 0x00;

constexpr uint8_t CONCIERGE_ADDRESS1 = 0x00;
constexpr uint8_t CONCIERGE_ADDRESS2 = 0x00;
constexpr uint8_t CLEAR_BUS_COMMAND = 0x11;
constexpr uint8_t CONCIERGE_CALL_COMMAND = 0x22;
constexpr uint8_t CONCIERGE_UNLOCK_COMMAND = 0x90;

constexpr uint32_t DEFAULT_CALL_ALERT_DURATION_MS = 2000;
constexpr uint32_t DEFAULT_INTER_COMMAND_DELAY_DURATION_MS = 500;


class golmar_uno_component : public Component, public uart::UARTDevice {

#ifdef USE_BINARY_SENSOR
   SUB_BINARY_SENSOR(calling_alert)
#endif

#ifdef USE_BUTTON
   SUB_BUTTON(unlock_door)
#endif

#ifdef USE_SWITCH
   SUB_SWITCH(unlock_door)
#endif

#ifdef USE_LOCK
   SUB_LOCK(door_lock)
#endif

protected:
   uint8_t intercom_id_{};
   uint8_t concierge_id_{};
   size_t match_index_ = 0;

   void detect_incoming_call_(uint8_t byte);

   // Write a 4-byte payload to the bus (address1, address2, address3, command)
   void write_payload(uint8_t address1, uint8_t address2, uint8_t address3, uint8_t command);

   // Convenience wrapper to send a command to the concierge (uses `concierge_id_`)
   void write_concierge_command(uint8_t command);

   // Convenience wrapper to send a command to the intercom (uses `intercom_id_`)
   void write_intercom_command(uint8_t command);


public:

   void loop() override;
   void setup() override;
   void dump_config() override;
   void unlock();
   void clear_bus();
   void schedule_switch_off(uint32_t delay_ms);
   //void setup() override;
   void set_intercom_id(uint8_t intercom_id) { this->intercom_id_ = intercom_id; }
   void set_concierge_id(uint8_t concierge_id) { this->concierge_id_ = concierge_id; }
   
#ifdef USE_BINARY_SENSOR
   void set_calling_alert_binary_sensor_(binary_sensor::BinarySensor *calling_alert_binary_sensor) {
     this->calling_alert_binary_sensor_ = calling_alert_binary_sensor;
   }
#endif

#ifdef USE_BUTTON
    void set_unlock_door_button_(button::Button *unlock_door_button) {
       this->unlock_door_button_ = unlock_door_button;
    }
#endif

#ifdef USE_SWITCH
    void set_unlock_door_switch_(switch_::Switch *unlock_door_switch) {
       this->unlock_door_switch_ = unlock_door_switch;
    }
#endif

#ifdef USE_LOCK
   void set_door_lock_(lock::Lock *door_lock) { this->door_lock_ = door_lock; }
   void schedule_door_lock(uint32_t delay_ms);
#endif
};

}  // namespace esphome::golmar_uno