# Golmar UNO ESPHome Component

> **Language / Idioma:** [English](#english) | [Español](#español)

---

<a name="english"></a>
## English

### Description

ESPHome external component for integrating Golmar UNO intercom systems (T720/T540) with Home Assistant. This component enables:

- **Incoming call detection** - Binary sensor that activates when someone rings the intercom
- **Door unlock control** - Multiple entity types available:
  - **Button** - Simple momentary unlock action
  - **Switch** - Toggle with auto-off after configurable duration
  - **Lock** - Full lock entity with unlock, lock, and open actions

### Installation

Add the external component to your ESPHome configuration:

```yaml
external_components:
  - source: github://jesusvallejo/components-esphome/components@main
    components: [ golmar_uno ]
    refresh: 0s
```

### Hardware Requirements

- ESP32 board (tested with ESP32-C3)
- UART connection to Golmar UNO intercom system
- Protocol: 2600 baud, 8 data bits, EVEN parity, 1 stop bit

### Configuration

#### UART Setup

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

#### Component Configuration

```yaml
golmar_uno:
  intercom_id: 0x11          # Your intercom device ID
  concierge_id: 0x00         # Concierge/doorman ID
  call_alert_duration: 2s    # Duration for call alert (default: 2s)
  unlock_timeout: 1s         # Timeout waiting for confirmation (default: 1s)
  command_delay: 500ms       # Delay between commands (min: 500ms)
```

| Option | Required | Default | Description |
|--------|----------|---------|-------------|
| `intercom_id` | Yes | - | Your intercom unit ID (hex) |
| `concierge_id` | Yes | - | Concierge/doorman ID (hex) |
| `call_alert_duration` | No | 2s | How long the call sensor stays on |
| `unlock_timeout` | No | 1s | Timeout for confirmation responses |
| `command_delay` | No | 500ms | Delay between protocol commands (min 500ms) |

### Entities

#### Binary Sensor (Incoming Call)

```yaml
binary_sensor:
  - platform: golmar_uno
    name: "Intercom Call"
```

Detects incoming calls. The sensor turns on when a call is received and automatically turns off after `call_alert_duration`.

#### Button (Unlock Door)

```yaml
button:
  - platform: golmar_uno
    name: "Unlock Door"
```

Simple momentary action to unlock the door.

#### Switch (Unlock Door)

```yaml
switch:
  - platform: golmar_uno
    name: "Door Unlock"
```

Toggle switch that auto-turns off after 2 seconds. Useful for automations.

#### Lock (Door Lock)

```yaml
lock:
  - platform: golmar_uno
    name: "Front Door"
```

Full lock entity with:
- **Unlock** - Initiates unlock sequence, auto-locks after 10 seconds
- **Lock** - Immediately sets state to locked (no physical action)
- **Open** - Momentary unlock without auto-lock timer

### Complete Example

```yaml
esphome:
  name: intercom
  friendly_name: Golmar Intercom

esp32:
  board: esp32-c3-devkitm-1
  framework:
    type: esp-idf

logger:

api:
  encryption:
    key: "your-api-key"

ota:
  platform: esphome

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

uart:
  - id: intercom_uart
    tx_pin: GPIO10
    rx_pin: GPIO4
    baud_rate: 2600
    data_bits: 8
    parity: EVEN
    stop_bits: 1

golmar_uno:
  intercom_id: 0x11
  concierge_id: 0x00
  call_alert_duration: 2s
  unlock_timeout: 1s
  command_delay: 500ms

binary_sensor:
  - platform: golmar_uno
    name: "Incoming Call"

lock:
  - platform: golmar_uno
    name: "Front Door"
```

### Protocol Details

The Golmar UNO uses a 4-byte protocol over UART:

| Byte | Description |
|------|-------------|
| 1 | Address 1 |
| 2 | Address 2 |
| 3 | Device ID |
| 4 | Command |

**Commands:**
- `0x11` - Clear bus
- `0x01` - Confirmation
- `0x22` - Call concierge
- `0x37` - Incoming call
- `0x90` - Unlock door

### Troubleshooting

- **No call detection**: Verify `intercom_id` matches your device
- **Unlock not working**: Check `concierge_id` and UART wiring
- **Communication errors**: Ensure 500ms minimum between commands

---

<a name="español"></a>
## Español

### Descripción

Componente externo de ESPHome para integrar sistemas de portero automático Golmar UNO (T720/T540) con Home Assistant. Este componente permite:

- **Detección de llamadas entrantes** - Sensor binario que se activa cuando alguien llama al portero
- **Control de apertura de puerta** - Varios tipos de entidades disponibles:
  - **Botón** - Acción momentánea simple de apertura
  - **Interruptor** - Con apagado automático tras duración configurable
  - **Cerradura** - Entidad completa con acciones de abrir, cerrar y apertura momentánea

### Instalación

Añade el componente externo a tu configuración de ESPHome:

```yaml
external_components:
  - source: github://jesusvallejo/components-esphome/components@main
    components: [ golmar_uno ]
    refresh: 0s
```

### Requisitos de Hardware

- Placa ESP32 (probado con ESP32-C3)
- Conexión UART al sistema de portero Golmar UNO
- Protocolo: 2600 baudios, 8 bits de datos, paridad PAR, 1 bit de parada

### Configuración

#### Configuración UART

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

#### Configuración del Componente

```yaml
golmar_uno:
  intercom_id: 0x11          # ID de tu dispositivo de portero
  concierge_id: 0x00         # ID del conserje/portero
  call_alert_duration: 2s    # Duración de la alerta de llamada (por defecto: 2s)
  unlock_timeout: 1s         # Tiempo de espera para confirmación (por defecto: 1s)
  command_delay: 500ms       # Retardo entre comandos (mín: 500ms)
```

| Opción | Requerido | Por defecto | Descripción |
|--------|-----------|-------------|-------------|
| `intercom_id` | Sí | - | ID de tu unidad de portero (hex) |
| `concierge_id` | Sí | - | ID del conserje/portero (hex) |
| `call_alert_duration` | No | 2s | Duración del sensor de llamada activo |
| `unlock_timeout` | No | 1s | Tiempo de espera para respuestas de confirmación |
| `command_delay` | No | 500ms | Retardo entre comandos del protocolo (mín 500ms) |

### Entidades

#### Sensor Binario (Llamada Entrante)

```yaml
binary_sensor:
  - platform: golmar_uno
    name: "Llamada Portero"
```

Detecta llamadas entrantes. El sensor se activa cuando se recibe una llamada y se desactiva automáticamente después de `call_alert_duration`.

#### Botón (Abrir Puerta)

```yaml
button:
  - platform: golmar_uno
    name: "Abrir Puerta"
```

Acción momentánea simple para abrir la puerta.

#### Interruptor (Abrir Puerta)

```yaml
switch:
  - platform: golmar_uno
    name: "Apertura Puerta"
```

Interruptor que se apaga automáticamente después de 2 segundos. Útil para automatizaciones.

#### Cerradura (Cerradura de Puerta)

```yaml
lock:
  - platform: golmar_uno
    name: "Puerta Principal"
```

Entidad de cerradura completa con:
- **Desbloquear** - Inicia secuencia de apertura, se bloquea automáticamente después de 10 segundos
- **Bloquear** - Establece inmediatamente el estado a bloqueado (sin acción física)
- **Abrir** - Apertura momentánea sin temporizador de auto-bloqueo

### Ejemplo Completo

```yaml
esphome:
  name: portero
  friendly_name: Portero Golmar

esp32:
  board: esp32-c3-devkitm-1
  framework:
    type: esp-idf

logger:

api:
  encryption:
    key: "tu-clave-api"

ota:
  platform: esphome

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

uart:
  - id: intercom_uart
    tx_pin: GPIO10
    rx_pin: GPIO4
    baud_rate: 2600
    data_bits: 8
    parity: EVEN
    stop_bits: 1

golmar_uno:
  intercom_id: 0x11
  concierge_id: 0x00
  call_alert_duration: 2s
  unlock_timeout: 1s
  command_delay: 500ms

binary_sensor:
  - platform: golmar_uno
    name: "Llamada Entrante"

lock:
  - platform: golmar_uno
    name: "Puerta Principal"
```

### Detalles del Protocolo

El Golmar UNO utiliza un protocolo de 4 bytes sobre UART:

| Byte | Descripción |
|------|-------------|
| 1 | Dirección 1 |
| 2 | Dirección 2 |
| 3 | ID del dispositivo |
| 4 | Comando |

**Comandos:**
- `0x11` - Limpiar bus
- `0x01` - Confirmación
- `0x22` - Llamar a conserjería
- `0x37` - Llamada entrante
- `0x90` - Abrir puerta

### Solución de Problemas

- **No detecta llamadas**: Verifica que `intercom_id` coincida con tu dispositivo
- **La apertura no funciona**: Comprueba `concierge_id` y el cableado UART
- **Errores de comunicación**: Asegura un mínimo de 500ms entre comandos

---

## License / Licencia

MIT License
