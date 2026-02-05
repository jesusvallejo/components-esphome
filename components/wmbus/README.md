# wM-Bus (Wireless M-Bus) ESPHome Component

[![ESPHome](https://img.shields.io/badge/ESPHome-Component-blue.svg)](https://esphome.io/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

> **Language / Idioma:** English | [Español](README_ES.md)

## Description

ESPHome external component for receiving and decoding **Wireless M-Bus (wM-Bus)** telegrams from smart meters using a **CC1101** radio module. This component enables real-time monitoring of utility meters (water, gas, electricity, heat) directly in Home Assistant.

### Features

- 📡 **CC1101 Radio Support** - Native SPI communication with CC1101 transceiver
- 🔐 **AES Decryption** - Support for encrypted meter data (Mode 5, Mode 7)
- 📊 **80+ Meter Types** - Built-in drivers for popular meter brands
- ⚡ **Real-time Reading** - Instant meter data updates
- 🔧 **Flexible Configuration** - Configurable frequency, sync mode, and LED indicators
- 📝 **Text & Numeric Sensors** - Support for both sensor types

## Table of Contents

- [Installation](#installation)
- [Hardware Requirements](#hardware-requirements)
- [Wiring Diagram](#wiring-diagram)
- [Configuration](#configuration)
  - [Component Configuration](#component-configuration)
  - [Configuration Options](#configuration-options)
- [Sensors](#sensors)
  - [Numeric Sensors](#numeric-sensors)
  - [Text Sensors](#text-sensors)
- [Supported Meters](#supported-meters)
- [Complete Example](#complete-example)
- [Finding Meter Information](#finding-meter-information)
- [Encryption Keys](#encryption-keys)
- [Troubleshooting](#troubleshooting)
- [FAQ](#faq)
- [License](#license)

## Installation

Add the external component to your ESPHome configuration:

```yaml
external_components:
  - source: github://jesusvallejo/components-esphome@main
    components: [ wmbus ]
    refresh: 0s
```

## Hardware Requirements

| Component | Specification |
|-----------|---------------|
| **Microcontroller** | ESP32 (tested platform) |
| **Radio Module** | CC1101 (868 MHz for Europe, 915 MHz for US) |
| **Connection** | SPI interface |
| **Optional** | Status LED |

### Recommended Hardware

- **ESP32** boards (tested and recommended)
- **CC1101** module with 868.95 MHz (Europe) or 915 MHz (US/Americas)
- External antenna for better reception range

## Wiring Diagram

### Default Pinout (ESP32)

```
ESP32                    CC1101 Module
┌─────────┐              ┌─────────────┐
│         │              │             │
│  GPIO13 ┼──────────────┼── MOSI      │
│  GPIO12 ┼──────────────┼── MISO      │
│  GPIO14 ┼──────────────┼── CLK       │
│   GPIO2 ┼──────────────┼── CS        │
│   GPIO5 ┼──────────────┼── GDO0      │
│   GPIO4 ┼──────────────┼── GDO2      │
│     3V3 ┼──────────────┼── VCC       │
│     GND ┼──────────────┼── GND       │
│         │              │             │
└─────────┘              └─────────────┘
```

> ⚠️ **Important**: The CC1101 operates at 3.3V. Do NOT connect to 5V!

## Configuration

### Component Configuration

```yaml
# Time component is required
time:
  - platform: homeassistant
    id: homeassistant_time

# wM-Bus component
wmbus:
  mosi_pin: GPIO13
  miso_pin: GPIO12
  clk_pin: GPIO14
  cs_pin: GPIO2
  gdo0_pin: GPIO5
  gdo2_pin: GPIO4
  frequency: 868.950      # MHz - Europe: 868.950, US: 915.000
  led_pin: GPIO15         # Optional: LED for receive indication
  led_blink_time: 200ms   # LED blink duration
  log_all: false          # Log all received telegrams
  all_drivers: false      # Load all drivers (increases flash usage)
  sync_mode: false        # Synchronous mode
```

### Configuration Options

| Option | Required | Default | Description |
|--------|----------|---------|-------------|
| `mosi_pin` | No | `GPIO13` | SPI MOSI pin |
| `miso_pin` | No | `GPIO12` | SPI MISO pin |
| `clk_pin` | No | `GPIO14` | SPI Clock pin |
| `cs_pin` | No | `GPIO2` | SPI Chip Select pin |
| `gdo0_pin` | No | `GPIO5` | CC1101 GDO0 pin |
| `gdo2_pin` | No | `GPIO4` | CC1101 GDO2 pin |
| `frequency` | No | `868.950` | Radio frequency in MHz (300-928 MHz) |
| `led_pin` | No | - | GPIO pin for status LED |
| `led_blink_time` | No | `200ms` | LED blink duration on telegram receive |
| `log_all` | No | `false` | Log all received telegrams (debug) |
| `all_drivers` | No | `false` | Load all meter drivers (increases flash) |
| `sync_mode` | No | `false` | Use synchronous reception mode |

## Sensors

### Numeric Sensors

```yaml
sensor:
  - platform: wmbus
    meter_id: 0x12345678
    type: "multical21"
    key: ""                    # 32 hex characters if encrypted
    sensors:
      - name: "Water Total"
        field: "total"
        unit_of_measurement: "m³"
        accuracy_decimals: 3
        device_class: water
        state_class: total_increasing
        
      - name: "Water Flow"
        field: "flow"
        unit_of_measurement: "m³/h"
        accuracy_decimals: 3
```

#### Sensor Options

| Option | Required | Default | Description |
|--------|----------|---------|-------------|
| `meter_id` | **Yes** | - | Meter ID in hex format (e.g., `0x12345678`) |
| `type` | **Yes** | - | Meter driver type (see [Supported Meters](#supported-meters)) |
| `key` | No | `""` | AES decryption key (32 hex characters) |
| `sensors` | **Yes** | - | List of sensors to create |

#### Sensor Fields

| Option | Required | Default | Description |
|--------|----------|---------|-------------|
| `name` | **Yes** | - | Sensor name in Home Assistant |
| `field` | No | name | Field name from meter driver |
| `unit_of_measurement` | **Yes** | - | Unit for the sensor value |
| `accuracy_decimals` | No | `3` | Number of decimal places |
| `device_class` | No | - | Home Assistant device class |
| `state_class` | No | - | Home Assistant state class |

### Text Sensors

```yaml
text_sensor:
  - platform: wmbus
    meter_id: 0x12345678
    type: "multical21"
    sensors:
      - name: "Water Meter Status"
        field: "status"
        
      - name: "Current Date"
        field: "current_date"
```

## Supported Meters

The component includes drivers for 80+ meter types. Here are some commonly used ones:

### Water Meters

| Driver | Manufacturer | Models |
|--------|--------------|--------|
| `multical21` | Kamstrup | Multical 21 |
| `iperl` | Sensus | iPERL |
| `hydrus` | Diehl | Hydrus |
| `izar` | Diehl/Sappel | IZAR RC 868 |
| `apator162` | Apator | AT-WMBUS-16-2 |
| `flowiq2200` | Kamstrup | flowIQ 2200 |
| `itron` | Itron | Various |
| `minomess` | Minol | Minomess |

### Heat/Energy Meters

| Driver | Manufacturer | Models |
|--------|--------------|--------|
| `kamheat` | Kamstrup | Heat meters |
| `sharky` | Diehl | Sharky 774/775 |
| `sensostar` | Engelmann | SensoStar |
| `qheat` | Qundis | Q heat |
| `compact5` | Kamstrup | MULTICAL 302/403 |
| `ultraheat` | Landis+Gyr | Ultraheat |
| `hydrocalm3` | Diehl | Hydrocalm 3 |

### Electricity Meters

| Driver | Manufacturer | Models |
|--------|--------------|--------|
| `omnipower` | Kamstrup | OMNIPOWER |
| `amiplus` | Apator | Amiplus |
| `em24` | Carlo Gavazzi | EM24 |
| `iem3000` | Schneider Electric | iEM3000 |
| `ebzwmbe` | EBZ | WMB-E01 |

### Gas Meters

| Driver | Manufacturer | Models |
|--------|--------------|--------|
| `unismart` | Apator | Unismart |
| `bfw240radio` | BFW | 240 Radio |

### Heat Cost Allocators

| Driver | Manufacturer | Models |
|--------|--------------|--------|
| `qcaloric` | Qundis | Q caloric |
| `fhkvdataiii` | Techem | FHKV data III |
| `fhkvdataiv` | Techem | FHKV data IV |
| `aventieshca` | Engelmann | Aventies |

### Smoke Detectors

| Driver | Manufacturer | Models |
|--------|--------------|--------|
| `ei6500` | Ei Electronics | Ei650 |
| `lansensm` | Lansen | Smoke detectors |

### Temperature/Humidity Sensors

| Driver | Manufacturer | Models |
|--------|--------------|--------|
| `rfmamb` | Diehl | RF Module Amb |
| `lansenth` | Lansen | Temperature/Humidity |
| `munia` | Munia | T/H sensors |

> 💡 **Tip**: Set `all_drivers: true` to load all drivers if you're unsure which one to use. Check the logs to identify your meter type.

## Complete Example

```yaml
esphome:
  name: wmbus-receiver
  friendly_name: wM-Bus Receiver

esp32:
  board: esp32dev
  framework:
    type: esp-idf

logger:
  level: DEBUG

api:
  encryption:
    key: "your-api-key"

ota:
  platform: esphome

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

external_components:
  - source: github://jesusvallejo/components-esphome@main
    components: [ wmbus ]
    refresh: 0s

# Required time component
time:
  - platform: homeassistant
    id: homeassistant_time

# wM-Bus receiver
wmbus:
  mosi_pin: GPIO13
  miso_pin: GPIO12
  clk_pin: GPIO14
  cs_pin: GPIO2
  gdo0_pin: GPIO5
  gdo2_pin: GPIO4
  frequency: 868.950
  led_pin: GPIO15
  led_blink_time: 200ms
  log_all: true         # Enable for initial setup

# Water meter
sensor:
  - platform: wmbus
    meter_id: 0x12345678
    type: "multical21"
    key: "00112233445566778899AABBCCDDEEFF"  # If encrypted
    sensors:
      - name: "Water Consumption"
        field: "total"
        unit_of_measurement: "m³"
        accuracy_decimals: 3
        device_class: water
        state_class: total_increasing
        icon: "mdi:water"
        
      - name: "Water Target"
        field: "target"
        unit_of_measurement: "m³"
        accuracy_decimals: 3
        
      - name: "Water Flow Rate"
        field: "flow"
        unit_of_measurement: "m³/h"
        accuracy_decimals: 3
        icon: "mdi:water-pump"

# Heat meter
  - platform: wmbus
    meter_id: 0xAABBCCDD
    type: "sharky"
    sensors:
      - name: "Heat Energy"
        field: "total_energy"
        unit_of_measurement: "kWh"
        device_class: energy
        state_class: total_increasing
        
      - name: "Heat Power"
        field: "power"
        unit_of_measurement: "kW"
        device_class: power

# Text sensors for status
text_sensor:
  - platform: wmbus
    meter_id: 0x12345678
    type: "multical21"
    sensors:
      - name: "Water Meter Status"
        field: "status"
        icon: "mdi:information"
```

## Finding Meter Information

### Step 1: Enable Logging

```yaml
logger:
  level: DEBUG

wmbus:
  log_all: true
  all_drivers: true
```

### Step 2: Check Logs

When a telegram is received, you'll see output like:

```
[D][wmbus:042]: Telegram received
[D][wmbus:043]: Meter ID: 12345678
[D][wmbus:044]: Type: multical21
[D][wmbus:045]: Manufacturer: KAM
[D][wmbus:046]: Fields: total=123.456 m³, target=120.000 m³
```

### Step 3: Configure Sensors

Use the meter ID, type, and field names from the logs to configure your sensors.

## Encryption Keys

Many meters use AES encryption. To decrypt the data:

1. **Contact your utility provider** - They may provide the key
2. **Check the meter documentation** - Some keys are printed on the meter
3. **Use wmbusmeters on a PC** - To identify if encryption is used

### Key Format

Keys must be exactly 32 hexadecimal characters (16 bytes):

```yaml
key: "00112233445566778899AABBCCDDEEFF"
```

### No Encryption

If the meter is not encrypted, leave the key empty or omit it:

```yaml
key: ""
# or simply don't include the key option
```

## Troubleshooting

### No Telegrams Received

1. ✅ Check SPI wiring (MOSI, MISO, CLK, CS)
2. ✅ Verify GDO0 and GDO2 connections
3. ✅ Ensure CC1101 is 3.3V (NOT 5V!)
4. ✅ Check frequency matches your region (868 MHz Europe, 915 MHz US)
5. ✅ Move closer to the meter for testing
6. ✅ Enable `log_all: true` to see all received telegrams

### Wrong/No Data

1. ✅ Verify meter ID is correct (check logs)
2. ✅ Verify meter type/driver is correct
3. ✅ Check if encryption key is required and correct
4. ✅ Verify field names match the driver output

### CC1101 Not Found

1. ✅ Check SPI connections
2. ✅ Verify CS pin is correctly configured
3. ✅ Check power supply (3.3V, sufficient current)
4. ✅ Try different SPI pins

### Intermittent Reception

1. ✅ Use external antenna for better range
2. ✅ Reduce distance to meters
3. ✅ Check for radio interference
4. ✅ Ensure stable power supply

### Debug Logging

```yaml
logger:
  level: VERBOSE
  logs:
    wmbus: DEBUG
    cc1101: DEBUG
```

## FAQ

**Q: What frequency should I use?**
A: Europe uses 868.95 MHz, North/South America uses 915 MHz. Check local regulations.

**Q: How do I find my meter ID?**
A: Enable `log_all: true` and check the logs. The ID is usually printed on the meter as well.

**Q: Can I receive multiple meters?**
A: Yes! Add multiple sensor configurations with different `meter_id` values.

**Q: What's the maximum range?**
A: Typically 50-100 meters indoors, more with external antenna in direct line of sight.

**Q: Do I need the encryption key?**
A: Only if your meter transmits encrypted data. Check with your utility provider.

**Q: Why is `all_drivers: true` slow to compile?**
A: It includes all 80+ meter drivers. Once you identify your meter type, disable it and specify only the needed driver.

**Q: How often do meters transmit?**
A: Varies by meter. Typically every few seconds to every few minutes.

**Q: Is ESP8266 supported?**
A: Only ESP32 has been tested. ESP8266 may work but is not supported.

## Credits

- Original wM-Bus implementation forked from [@SzczepanLeon](https://github.com/SzczepanLeon)
- CC1101 integration and maintenance by [@jesusvallejo](https://github.com/jesusvallejo)
- Based on [wmbusmeters](https://github.com/wmbusmeters/wmbusmeters) project

## License

MIT License

Copyright (c) 2024-2026 jesusvallejo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Authors

- **[@SzczepanLeon](https://github.com/SzczepanLeon)** - Original wM-Bus implementation
- **[@jesusvallejo](https://github.com/jesusvallejo)** - CC1101 integration and repository maintainer

## Acknowledgments

- Thanks to the ESPHome community
- Thanks to the wmbusmeters project for meter driver implementations
- Thanks to all contributors and testers
