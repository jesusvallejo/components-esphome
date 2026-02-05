# Componente Golmar UNO para ESPHome

[![ESPHome](https://img.shields.io/badge/ESPHome-Componente-blue.svg)](https://esphome.io/)
[![Licencia: MIT](https://img.shields.io/badge/Licencia-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

> **Language / Idioma:** [English](README.md) | Español

## Descripción

Componente externo de ESPHome para integrar **sistemas de portero automático Golmar UNO** (T720/T540) con Home Assistant. Este componente proporciona una integración perfecta del sistema de portero de tu edificio con tu hogar inteligente.

### Características

- 📞 **Detección de llamadas entrantes** - Sensor binario que se activa cuando alguien llama al portero
- 🔓 **Control de apertura de puerta** - Múltiples tipos de entidades disponibles:
  - **Botón** - Acción momentánea simple de apertura
  - **Interruptor** - Con apagado automático tras duración configurable
  - **Cerradura** - Entidad completa con acciones de abrir, cerrar y apertura momentánea
- 🔌 **Protocolo UART** - Comunicación directa con el sistema de portero
- ⚡ **Respuesta en tiempo real** - Notificación instantánea de llamadas y operaciones de puerta

## Tabla de Contenidos

- [Instalación](#instalación)
- [Requisitos de Hardware](#requisitos-de-hardware)
- [Diagrama de Conexión](#diagrama-de-conexión)
- [Configuración](#configuración)
  - [Configuración UART](#configuración-uart)
  - [Configuración del Componente](#configuración-del-componente)
  - [Opciones de Configuración](#opciones-de-configuración)
- [Entidades](#entidades)
  - [Sensor Binario (Llamada Entrante)](#sensor-binario-llamada-entrante)
  - [Botón (Abrir Puerta)](#botón-abrir-puerta)
  - [Interruptor (Abrir Puerta)](#interruptor-abrir-puerta)
  - [Cerradura (Cerradura de Puerta)](#cerradura-cerradura-de-puerta)
- [Ejemplo Completo](#ejemplo-completo)
- [Detalles del Protocolo](#detalles-del-protocolo)
- [Automatizaciones en Home Assistant](#automatizaciones-en-home-assistant)
- [Solución de Problemas](#solución-de-problemas)
- [Preguntas Frecuentes](#preguntas-frecuentes)
- [Licencia](#licencia)

## Instalación

Añade el componente externo a tu configuración de ESPHome:

```yaml
external_components:
  - source: github://jesusvallejo/components-esphome@main
    components: [ golmar_uno ]
    refresh: 0s
```

## Requisitos de Hardware

| Componente | Especificación |
|------------|----------------|
| **Microcontrolador** | Placa ESP32 (probado con ESP32-C3) |
| **Conexión** | UART al sistema de portero Golmar UNO |
| **Protocolo** | 2600 baudios, 8 bits de datos, paridad PAR, 1 bit de parada |

### Modelos de Portero Compatibles

- Golmar T720
- Golmar T540
- Otros sistemas compatibles con Golmar UNO

## Diagrama de Conexión

```
ESP32                    Portero Golmar UNO
┌─────────┐              ┌─────────────────┐
│         │              │                 │
│   TX ───┼──────────────┼─── RX           │
│   RX ───┼──────────────┼─── TX           │
│  GND ───┼──────────────┼─── GND          │
│         │              │                 │
└─────────┘              └─────────────────┘
```

> ⚠️ **Advertencia**: Asegúrate de usar los niveles de voltaje correctos. El sistema Golmar UNO puede requerir adaptación de niveles si no opera a 3.3V.

## Configuración

### Configuración UART

Configura el bus UART con los parámetros correctos para la comunicación con Golmar UNO:

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

### Configuración del Componente

```yaml
golmar_uno:
  intercom_id: 0x11          # ID de tu dispositivo de portero
  concierge_id: 0x00         # ID del conserje/portero
  call_alert_duration: 2s    # Duración de la alerta de llamada (por defecto: 2s)
  unlock_timeout: 1s         # Tiempo de espera para confirmación (por defecto: 1s)
  command_delay: 500ms       # Retardo entre comandos (mín: 500ms)
```

### Opciones de Configuración

| Opción | Requerido | Por Defecto | Descripción |
|--------|-----------|-------------|-------------|
| `intercom_id` | **Sí** | - | ID de tu unidad de portero en formato hexadecimal (ej: `0x11`) |
| `concierge_id` | No | `0x00` | ID del conserje/portero en formato hexadecimal |
| `call_alert_duration` | No | `2s` | Tiempo que el sensor binario de llamada permanece activo tras detectar una llamada |
| `unlock_timeout` | No | `1s` | Tiempo de espera para respuestas de confirmación del portero |
| `command_delay` | No | `500ms` | Retardo entre comandos del protocolo (mínimo 500ms requerido por el protocolo) |

### Encontrar el ID de tu Portero

Para encontrar tu `intercom_id`:

1. Habilita el registro de depuración en ESPHome:
   ```yaml
   logger:
     level: DEBUG
   ```
2. Llama a tu portero desde el panel de la calle
3. Revisa los registros buscando paquetes entrantes - el ID del dispositivo será visible

## Entidades

### Sensor Binario (Llamada Entrante)

```yaml
binary_sensor:
  - platform: golmar_uno
    name: "Llamada Portero"
    device_class: occupancy  # Opcional: para mejor integración con Home Assistant
```

Detecta llamadas entrantes del portero. El sensor:
- Se activa (**ON**) cuando se recibe una llamada
- Se desactiva automáticamente (**OFF**) después de `call_alert_duration`

**Casos de uso:**
- Activar notificaciones en tu teléfono
- Hacer parpadear las luces cuando alguien llama
- Mostrar información de la llamada en un panel de control

### Botón (Abrir Puerta)

```yaml
button:
  - platform: golmar_uno
    name: "Abrir Puerta"
    icon: "mdi:door-open"  # Opcional
```

Acción momentánea simple para abrir la puerta. Pulsa una vez para enviar el comando de apertura.

**Casos de uso:**
- Apertura rápida desde el panel de Home Assistant
- Integración con asistentes de voz ("Hey Google, abre la puerta")

### Interruptor (Abrir Puerta)

```yaml
switch:
  - platform: golmar_uno
    name: "Apertura Puerta"
    icon: "mdi:door"  # Opcional
```

Interruptor que:
- Se activa (**ON**) cuando se acciona
- Se desactiva automáticamente (**OFF**) después de 2 segundos
- Útil para automatizaciones que necesitan retroalimentación de estado

### Cerradura (Cerradura de Puerta)

```yaml
lock:
  - platform: golmar_uno
    name: "Puerta Principal"
```

Entidad de cerradura completa con controles completos:

| Acción | Comportamiento |
|--------|----------------|
| **Desbloquear** | Inicia la secuencia de apertura, vuelve automáticamente al estado bloqueado después de 10 segundos |
| **Bloquear** | Establece inmediatamente el estado a bloqueado (sin acción física enviada al portero) |
| **Abrir** | Apertura momentánea sin temporizador de auto-bloqueo |

**Casos de uso:**
- Integración con el panel de cerraduras de Home Assistant
- Soporte para Apple HomeKit / Google Home (a través de Home Assistant)

## Ejemplo Completo

```yaml
esphome:
  name: portero
  friendly_name: Portero Golmar

esp32:
  board: esp32-c3-devkitm-1
  framework:
    type: esp-idf

# Habilitar registro
logger:
  level: DEBUG

# Habilitar API de Home Assistant
api:
  encryption:
    key: "tu-clave-api-aqui"

# Habilitar actualizaciones OTA
ota:
  platform: esphome
  password: "tu-contraseña-ota"

# Configuración WiFi
wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
  
  # Habilitar punto de acceso de respaldo
  ap:
    ssid: "Portero Fallback"
    password: "contraseña-respaldo"

# Servidor web (opcional)
web_server:
  port: 80

# Componente externo
external_components:
  - source: github://jesusvallejo/components-esphome@main
    components: [ golmar_uno ]
    refresh: 0s

# Configuración UART
uart:
  - id: intercom_uart
    tx_pin: GPIO10
    rx_pin: GPIO4
    baud_rate: 2600
    data_bits: 8
    parity: EVEN
    stop_bits: 1

# Componente Golmar UNO
golmar_uno:
  intercom_id: 0x11
  concierge_id: 0x00
  call_alert_duration: 2s
  unlock_timeout: 1s
  command_delay: 500ms

# Sensor binario para llamadas entrantes
binary_sensor:
  - platform: golmar_uno
    name: "Llamada Entrante"
    device_class: occupancy

# Entidad de cerradura para control de puerta
lock:
  - platform: golmar_uno
    name: "Puerta Principal"

# Opcional: Botón para apertura rápida
button:
  - platform: golmar_uno
    name: "Apertura Rápida"
    icon: "mdi:door-open"
```

## Detalles del Protocolo

El Golmar UNO utiliza un protocolo de 4 bytes sobre UART:

| Byte | Descripción | Ejemplo |
|------|-------------|---------|
| 1 | Dirección 1 | `0x00` |
| 2 | Dirección 2 | `0x00` |
| 3 | ID del Dispositivo | `0x11` (tu portero) |
| 4 | Comando | `0x37` (llamada entrante) |

### Comandos del Protocolo

| Comando | Valor Hex | Descripción |
|---------|-----------|-------------|
| Limpiar Bus | `0x11` | Limpia el bus de comunicación |
| Confirmación | `0x01` | Respuesta de reconocimiento |
| Llamar Conserjería | `0x22` | Inicia llamada a conserjería |
| Llamada Entrante | `0x37` | Indica que alguien está llamando |
| Abrir Puerta | `0x90` | Envía comando de apertura de puerta |

### Flujo de Comunicación

```
1. Llamada Entrante Detectada
   [PORTERO] --> [0x00, 0x00, 0x11, 0x37] --> [ESP32]
   
2. Secuencia de Apertura de Puerta
   [ESP32] --> [0x00, 0x00, 0x00, 0x22] --> [PORTERO]  (Llamar conserjería)
   [PORTERO] --> [0x00, 0x00, 0x00, 0x01] --> [ESP32]  (Confirmación)
   [ESP32] --> [0x00, 0x00, 0x00, 0x90] --> [PORTERO]  (Abrir)
   [PORTERO] --> [0x00, 0x00, 0x00, 0x01] --> [ESP32]  (Confirmación)
   [ESP32] --> [0x00, 0x00, 0x00, 0x11] --> [PORTERO]  (Limpiar bus)
```

## Automatizaciones en Home Assistant

### Enviar notificación cuando alguien llama

```yaml
automation:
  - alias: "Notificación de Llamada al Portero"
    trigger:
      - platform: state
        entity_id: binary_sensor.llamada_entrante
        to: "on"
    action:
      - service: notify.mobile_app_tu_telefono
        data:
          title: "🔔 Timbre"
          message: "¡Alguien está en la puerta!"
          data:
            actions:
              - action: "ABRIR_PUERTA"
                title: "Abrir Puerta"
```

### Auto-apertura en horarios específicos

```yaml
automation:
  - alias: "Auto Apertura Durante Horario de Entregas"
    trigger:
      - platform: state
        entity_id: binary_sensor.llamada_entrante
        to: "on"
    condition:
      - condition: time
        after: "09:00:00"
        before: "18:00:00"
      - condition: state
        entity_id: input_boolean.esperando_entrega
        state: "on"
    action:
      - delay: 2
      - service: lock.unlock
        target:
          entity_id: lock.puerta_principal
```

## Solución de Problemas

### No detecta llamadas

1. ✅ Verifica que `intercom_id` coincida con tu dispositivo
2. ✅ Comprueba el cableado UART (TX/RX, GND)
3. ✅ Asegúrate de que la velocidad sea 2600 baudios
4. ✅ Verifica que la paridad esté configurada como EVEN (PAR)
5. ✅ Habilita el registro DEBUG para ver los paquetes en bruto

### La apertura no funciona

1. ✅ Verifica la configuración de `concierge_id`
2. ✅ Comprueba que el pin TX esté correctamente conectado
3. ✅ Asegura un mínimo de 500ms entre comandos
4. ✅ Busca respuestas de confirmación en los registros

### Errores de comunicación

1. ✅ Asegura un retardo mínimo de 500ms entre comandos (`command_delay`)
2. ✅ Comprueba si hay conexiones sueltas
3. ✅ Verifica que los niveles de voltaje sean compatibles
4. ✅ Intenta aumentar `unlock_timeout` si las confirmaciones son lentas

### Registro de Depuración

Habilita el registro detallado para diagnosticar problemas:

```yaml
logger:
  level: VERBOSE
  logs:
    golmar_uno: DEBUG
```

## Preguntas Frecuentes

**P: ¿Puedo usar esto con ESP8266?**
R: El componente está diseñado para ESP32. ESP8266 podría funcionar pero no ha sido probado.

**P: ¿Qué pasa si no conozco mi intercom_id?**
R: Habilita el registro de depuración y llama al portero. El ID aparecerá en los registros.

**P: ¿Puedo tener múltiples porteros?**
R: Sí, puedes configurar múltiples instancias con diferentes valores de `intercom_id`.

**P: ¿Hay algún retardo al abrir la puerta?**
R: El retardo mínimo del protocolo es de 500ms entre comandos, así que espera ~1-2 segundos en total.

## Licencia

Este proyecto está licenciado bajo la Licencia MIT - consulta el archivo [LICENSE](../../LICENSE) para más detalles.

## Autor

- **[@jesusvallejo](https://github.com/jesusvallejo)**

## Agradecimientos

- Gracias a la comunidad de ESPHome por el excelente framework
- A Golmar por sus sistemas de portero automático
