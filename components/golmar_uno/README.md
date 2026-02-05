# Golmar UNO ESPHome Component

[![ESPHome](https://img.shields.io/badge/ESPHome-Component-blue.svg)](https://esphome.io/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

> **Language / Idioma:** English | [Español](README_ES.md)

## Description

ESPHome external component for integrating **Golmar UNO intercom systems** (T720/T540) with Home Assistant. This component provides seamless integration of your building's intercom system with your smart home.

### Features

- 📞 **Incoming call detection** - Binary sensor that activates when someone rings the intercom
- 🔓 **Door unlock control** - Multiple entity types available:
  - **Button** - Simple momentary unlock action
  - **Switch** - Toggle with auto-off after configurable duration
  - **Lock** - Full lock entity with unlock, lock, and open actions
- 🔌 **UART Protocol** - Direct communication with the intercom system
- ⚡ **Real-time response** - Instant notification of calls and door operations

## Table of Contents

- [Installation](#installation)
- [Hardware Requirements](#hardware-requirements)
- [Wiring Diagram](#wiring-diagram)
- [Configuration](#configuration)
  - [UART Setup](#uart-setup)
  - [Component Configuration](#component-configuration)
  - [Configuration Options](#configuration-options)
- [Entities](#entities)
  - [Binary Sensor (Incoming Call)](#binary-sensor-incoming-call)
  - [Button (Unlock Door)](#button-unlock-door)
  - [Switch (Unlock Door)](#switch-unlock-door)
  - [Lock (Door Lock)](#lock-door-lock)
- [Complete Example](#complete-example)
- [Protocol Details](#protocol-details)
- [Home Assistant Automations](#home-assistant-automations)
- [Troubleshooting](#troubleshooting)
- [FAQ](#faq)
- [License](#license)

## Installation

Add the external component to your ESPHome configuration:

```yaml
external_components:
  - source: github://jesusvallejo/components-esphome@main
    components: [ golmar_uno ]
    refresh: 0s
```

## Hardware Requirements

| Component | Specification |
|-----------|---------------|
| **Microcontroller** | ESP32 board (tested with ESP32-C3) |
| **Connection** | UART to Golmar UNO intercom system |
| **Protocol** | 2600 baud, 8 data bits, EVEN parity, 1 stop bit |

### Supported Intercom Models

- Golmar T720
- Golmar T540
- Other Golmar UNO compatible systems

## Wiring Diagram

```
ESP32                    Golmar UNO Intercom
┌─────────┐              ┌─────────────────┐
│         │              │                 │
│   TX ───┼──────────────┼─── RX           │
│   RX ───┼──────────────┼─── TX           │
│  GND ───┼──────────────┼─── GND          │
│         │              │                 │
└─────────┘              └─────────────────┘
```

> ⚠️ **Warning**: Make sure to use proper voltage levels. The Golmar UNO system may require level shifting if not operating at 3.3V.

## Configuration

### UART Setup

Configure the UART bus with the correct parameters for Golmar UNO communication:

```yaml
uart:
  - id: intercom_uart
    tx_pin: GPIO10
    rx_pin: GPIO4
    baud_rate: 2600
    data_bits: 8
    parity: EVEN
    stop_bits: 1
```

### Component Configuration

```yaml
golmar_uno:
  intercom_id: 0x11          # Your intercom device ID
  concierge_id: 0x00         # Concierge/doorman ID
  call_alert_duration: 2s    # Duration for call alert (default: 2s)
  unlock_timeout: 1s         # Timeout waiting for confirmation (default: 1s)
  command_delay: 500ms       # Delay between commands (min: 500ms)
```

### Configuration Options

| Option | Required | Default | Description |
|--------|----------|---------|-------------|
| `intercom_id` | **Yes** | - | Your intercom unit ID in hexadecimal format (e.g., `0x11`) |
| `concierge_id` | No | `0x00` | Concierge/doorman ID in hexadecimal format |
| `call_alert_duration` | No | `2s` | How long the call binary sensor stays on after detecting a call |
| `unlock_timeout` | No | `1s` | Timeout waiting for confirmation responses from the intercom |
| `command_delay` | No | `500ms` | Delay between protocol commands (minimum 500ms required by protocol) |

### Finding Your Intercom ID

To find your `intercom_id`:

1. Enable debug logging in ESPHome:
   ```yaml
   logger:
     level: DEBUG
   ```
2. Ring your intercom from the street panel
3. Check the logs for incoming packets - the device ID will be visible

## Entities

### Binary Sensor (Incoming Call)

```yaml
binary_sensor:
  - platform: golmar_uno
    name: "Intercom Call"
    device_class: occupancy  # Optional: for better Home Assistant integration
```

Detects incoming calls from the intercom. The sensor:
- Turns **ON** when a call is received
- Automatically turns **OFF** after `call_alert_duration`

**Use cases:**
- Trigger notifications on your phone
- Flash lights when someone rings
- Display caller information on a dashboard

### Button (Unlock Door)

```yaml
button:
  - platform: golmar_uno
    name: "Unlock Door"
    icon: "mdi:door-open"  # Optional
```

Simple momentary action to unlock the door. Press once to send the unlock command.

**Use cases:**
- Quick unlock from Home Assistant dashboard
- Voice assistant integration ("Hey Google, unlock the door")

### Switch (Unlock Door)

```yaml
switch:
  - platform: golmar_uno
    name: "Door Unlock"
    icon: "mdi:door"  # Optional
```

Toggle switch that:
- Turns **ON** when activated
- Automatically turns **OFF** after 2 seconds
- Useful for automations that need state feedback

### Lock (Door Lock)

```yaml
lock:
  - platform: golmar_uno
    name: "Front Door"
```

Full lock entity with comprehensive controls:

| Action | Behavior |
|--------|----------|
| **Unlock** | Initiates unlock sequence, automatically returns to locked state after 10 seconds |
| **Lock** | Immediately sets state to locked (no physical action sent to intercom) |
| **Open** | Momentary unlock without auto-lock timer |

**Use cases:**
- Integration with Home Assistant lock dashboard
- Apple HomeKit / Google Home lock support (via Home Assistant)

## Complete Example

```yaml
esphome:
  name: intercom
  friendly_name: Golmar Intercom

esp32:
  board: esp32-c3-devkitm-1
  framework:
    type: esp-idf

# Enable logging
logger:
  level: DEBUG

# Enable Home Assistant API
api:
  encryption:
    key: "your-api-key-here"

# Enable OTA updates
ota:
  platform: esphome
  password: "your-ota-password"

# WiFi configuration
wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
  
  # Enable fallback hotspot
  ap:
    ssid: "Intercom Fallback"
    password: "fallback-password"

# Web server (optional)
web_server:
  port: 80

# External component
external_components:
  - source: github://jesusvallejo/components-esphome@main
    components: [ golmar_uno ]
    refresh: 0s

# UART configuration
uart:
  - id: intercom_uart
    tx_pin: GPIO10
    rx_pin: GPIO4
    baud_rate: 2600
    data_bits: 8
    parity: EVEN
    stop_bits: 1

# Golmar UNO component
golmar_uno:
  intercom_id: 0x11
  concierge_id: 0x00
  call_alert_duration: 2s
  unlock_timeout: 1s
  command_delay: 500ms

# Binary sensor for incoming calls
binary_sensor:
  - platform: golmar_uno
    name: "Incoming Call"
    device_class: occupancy

# Lock entity for door control
lock:
  - platform: golmar_uno
    name: "Front Door"

# Optional: Button for quick unlock
button:
  - platform: golmar_uno
    name: "Quick Unlock"
    icon: "mdi:door-open"
```

## Protocol Details

The Golmar UNO uses a 4-byte protocol over UART:

| Byte | Description | Example |
|------|-------------|---------|
| 1 | Address 1 | `0x00` |
| 2 | Address 2 | `0x00` |
| 3 | Device ID | `0x11` (your intercom) |
| 4 | Command | `0x37` (incoming call) |

### Protocol Commands

| Command | Hex Value | Description |
|---------|-----------|-------------|
| Clear Bus | `0x11` | Clears the communication bus |
| Confirmation | `0x01` | Acknowledgment response |
| Call Concierge | `0x22` | Initiates call to concierge |
| Incoming Call | `0x37` | Indicates someone is calling |
| Unlock Door | `0x90` | Sends door unlock command |

### Communication Flow

```
1. Incoming Call Detected
   [INTERCOM] --> [0x00, 0x00, 0x11, 0x37] --> [ESP32]
   
2. Door Unlock Sequence
   [ESP32] --> [0x00, 0x00, 0x00, 0x22] --> [INTERCOM]  (Call concierge)
   [INTERCOM] --> [0x00, 0x00, 0x00, 0x01] --> [ESP32]  (Confirmation)
   [ESP32] --> [0x00, 0x00, 0x00, 0x90] --> [INTERCOM]  (Unlock)
   [INTERCOM] --> [0x00, 0x00, 0x00, 0x01] --> [ESP32]  (Confirmation)
   [ESP32] --> [0x00, 0x00, 0x00, 0x11] --> [INTERCOM]  (Clear bus)
```

## Home Assistant Automations

### Send notification when someone rings

```yaml
automation:
  - alias: "Intercom Call Notification"
    trigger:
      - platform: state
        entity_id: binary_sensor.incoming_call
        to: "on"
    action:
      - service: notify.mobile_app_your_phone
        data:
          title: "🔔 Doorbell"
          message: "Someone is at the door!"
          data:
            actions:
              - action: "UNLOCK_DOOR"
                title: "Unlock Door"
```

### Auto-unlock for specific times

```yaml
automation:
  - alias: "Auto Unlock During Delivery Hours"
    trigger:
      - platform: state
        entity_id: binary_sensor.incoming_call
        to: "on"
    condition:
      - condition: time
        after: "09:00:00"
        before: "18:00:00"
      - condition: state
        entity_id: input_boolean.expecting_delivery
        state: "on"
    action:
      - delay: 2
      - service: lock.unlock
        target:
          entity_id: lock.front_door
```

## Troubleshooting

### No call detection

1. ✅ Verify `intercom_id` matches your device
2. ✅ Check UART wiring (TX/RX, GND)
3. ✅ Ensure baud rate is set to 2600
4. ✅ Verify parity is set to EVEN
5. ✅ Enable DEBUG logging to see raw packets

### Unlock not working

1. ✅ Check `concierge_id` configuration
2. ✅ Verify TX pin is correctly connected
3. ✅ Ensure minimum 500ms between commands
4. ✅ Check for confirmation responses in logs

### Communication errors

1. ✅ Ensure 500ms minimum delay between commands (`command_delay`)
2. ✅ Check for loose connections
3. ✅ Verify voltage levels are compatible
4. ✅ Try increasing `unlock_timeout` if confirmations are slow

### Debug Logging

Enable verbose logging to diagnose issues:

```yaml
logger:
  level: VERBOSE
  logs:
    golmar_uno: DEBUG
```

## FAQ

**Q: Can I use this with ESP8266?**
A: The component is designed for ESP32. ESP8266 may work but is not tested.

**Q: What if I don't know my intercom_id?**
A: Enable debug logging and ring the intercom. The ID will appear in the logs.

**Q: Can I have multiple intercoms?**
A: Yes, you can configure multiple instances with different `intercom_id` values.

**Q: Is there any delay when unlocking?**
A: The minimum protocol delay is 500ms between commands, so expect ~1-2 seconds total.

## License

This project is licensed under the MIT License - see the [LICENSE](../../LICENSE) file for details.

## Author

- **[@jesusvallejo](https://github.com/jesusvallejo)**

## Acknowledgments

- Thanks to the ESPHome community for the excellent framework
- Golmar for their intercom systems
