# Componente wM-Bus (Wireless M-Bus) para ESPHome

[![ESPHome](https://img.shields.io/badge/ESPHome-Componente-blue.svg)](https://esphome.io/)
[![Licencia: MIT](https://img.shields.io/badge/Licencia-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

> **Language / Idioma:** [English](README.md) | Español

## Descripción

Componente externo de ESPHome para recibir y decodificar telegramas **Wireless M-Bus (wM-Bus)** de contadores inteligentes usando un módulo de radio **CC1101**. Este componente permite la monitorización en tiempo real de contadores de servicios (agua, gas, electricidad, calefacción) directamente en Home Assistant.

### Características

- 📡 **Soporte CC1101** - Comunicación SPI nativa con el transceptor CC1101
- 🔐 **Descifrado AES** - Soporte para datos cifrados de contadores (Modo 5, Modo 7)
- 📊 **Más de 80 Tipos de Contadores** - Drivers incorporados para marcas populares
- ⚡ **Lectura en Tiempo Real** - Actualizaciones instantáneas de datos del contador
- 🔧 **Configuración Flexible** - Frecuencia configurable, modo síncrono e indicadores LED
- 📝 **Sensores de Texto y Numéricos** - Soporte para ambos tipos de sensores

## Tabla de Contenidos

- [Instalación](#instalación)
- [Requisitos de Hardware](#requisitos-de-hardware)
- [Diagrama de Conexión](#diagrama-de-conexión)
- [Configuración](#configuración)
  - [Configuración del Componente](#configuración-del-componente)
  - [Opciones de Configuración](#opciones-de-configuración)
- [Sensores](#sensores)
  - [Sensores Numéricos](#sensores-numéricos)
  - [Sensores de Texto](#sensores-de-texto)
- [Contadores Compatibles](#contadores-compatibles)
- [Ejemplo Completo](#ejemplo-completo)
- [Encontrar Información del Contador](#encontrar-información-del-contador)
- [Claves de Cifrado](#claves-de-cifrado)
- [Solución de Problemas](#solución-de-problemas)
- [Preguntas Frecuentes](#preguntas-frecuentes)
- [Licencia](#licencia)

## Instalación

Añade el componente externo a tu configuración de ESPHome:

```yaml
external_components:
  - source: github://jesusvallejo/components-esphome@main
    components: [ wmbus ]
    refresh: 0s
```

## Requisitos de Hardware

| Componente | Especificación |
|------------|----------------|
| **Microcontrolador** | ESP32 (plataforma probada) |
| **Módulo de Radio** | CC1101 (868 MHz para Europa, 915 MHz para América) |
| **Conexión** | Interfaz SPI |
| **Opcional** | LED de estado |

### Hardware Recomendado

- Placas **ESP32** (probadas y recomendadas)
- Módulo **CC1101** con 868.95 MHz (Europa) o 915 MHz (América)
- Antena externa para mejor alcance de recepción

## Diagrama de Conexión

### Pinout por Defecto (ESP32)

```
ESP32                    Módulo CC1101
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

> ⚠️ **Importante**: El CC1101 opera a 3.3V. ¡NO conectar a 5V!

## Configuración

### Configuración del Componente

```yaml
# El componente time es requerido
time:
  - platform: homeassistant
    id: homeassistant_time

# Componente wM-Bus
wmbus:
  mosi_pin: GPIO13
  miso_pin: GPIO12
  clk_pin: GPIO14
  cs_pin: GPIO2
  gdo0_pin: GPIO5
  gdo2_pin: GPIO4
  frequency: 868.950      # MHz - Europa: 868.950, América: 915.000
  led_pin: GPIO15         # Opcional: LED para indicación de recepción
  led_blink_time: 200ms   # Duración del parpadeo del LED
  log_all: false          # Registrar todos los telegramas recibidos
  all_drivers: false      # Cargar todos los drivers (aumenta el uso de flash)
  sync_mode: false        # Modo síncrono
```

### Opciones de Configuración

| Opción | Requerido | Por Defecto | Descripción |
|--------|-----------|-------------|-------------|
| `mosi_pin` | No | `GPIO13` | Pin SPI MOSI |
| `miso_pin` | No | `GPIO12` | Pin SPI MISO |
| `clk_pin` | No | `GPIO14` | Pin de Reloj SPI |
| `cs_pin` | No | `GPIO2` | Pin de Selección de Chip SPI |
| `gdo0_pin` | No | `GPIO5` | Pin GDO0 del CC1101 |
| `gdo2_pin` | No | `GPIO4` | Pin GDO2 del CC1101 |
| `frequency` | No | `868.950` | Frecuencia de radio en MHz (300-928 MHz) |
| `led_pin` | No | - | Pin GPIO para LED de estado |
| `led_blink_time` | No | `200ms` | Duración del parpadeo del LED al recibir telegrama |
| `log_all` | No | `false` | Registrar todos los telegramas recibidos (depuración) |
| `all_drivers` | No | `false` | Cargar todos los drivers de contadores (aumenta flash) |
| `sync_mode` | No | `false` | Usar modo de recepción síncrono |

## Sensores

### Sensores Numéricos

```yaml
sensor:
  - platform: wmbus
    meter_id: 0x12345678
    type: "multical21"
    key: ""                    # 32 caracteres hex si está cifrado
    sensors:
      - name: "Agua Total"
        field: "total"
        unit_of_measurement: "m³"
        accuracy_decimals: 3
        device_class: water
        state_class: total_increasing
        
      - name: "Flujo de Agua"
        field: "flow"
        unit_of_measurement: "m³/h"
        accuracy_decimals: 3
```

#### Opciones del Sensor

| Opción | Requerido | Por Defecto | Descripción |
|--------|-----------|-------------|-------------|
| `meter_id` | **Sí** | - | ID del contador en formato hex (ej: `0x12345678`) |
| `type` | **Sí** | - | Tipo de driver del contador (ver [Contadores Compatibles](#contadores-compatibles)) |
| `key` | No | `""` | Clave de descifrado AES (32 caracteres hex) |
| `sensors` | **Sí** | - | Lista de sensores a crear |

#### Campos del Sensor

| Opción | Requerido | Por Defecto | Descripción |
|--------|-----------|-------------|-------------|
| `name` | **Sí** | - | Nombre del sensor en Home Assistant |
| `field` | No | name | Nombre del campo del driver del contador |
| `unit_of_measurement` | **Sí** | - | Unidad para el valor del sensor |
| `accuracy_decimals` | No | `3` | Número de decimales |
| `device_class` | No | - | Clase de dispositivo de Home Assistant |
| `state_class` | No | - | Clase de estado de Home Assistant |

### Sensores de Texto

```yaml
text_sensor:
  - platform: wmbus
    meter_id: 0x12345678
    type: "multical21"
    sensors:
      - name: "Estado Contador Agua"
        field: "status"
        
      - name: "Fecha Actual"
        field: "current_date"
```

## Contadores Compatibles

El componente incluye drivers para más de 80 tipos de contadores. Aquí están algunos de los más utilizados:

### Contadores de Agua

| Driver | Fabricante | Modelos |
|--------|------------|---------|
| `multical21` | Kamstrup | Multical 21 |
| `iperl` | Sensus | iPERL |
| `hydrus` | Diehl | Hydrus |
| `izar` | Diehl/Sappel | IZAR RC 868 |
| `apator162` | Apator | AT-WMBUS-16-2 |
| `flowiq2200` | Kamstrup | flowIQ 2200 |
| `itron` | Itron | Varios |
| `minomess` | Minol | Minomess |

### Contadores de Calor/Energía

| Driver | Fabricante | Modelos |
|--------|------------|---------|
| `kamheat` | Kamstrup | Contadores de calor |
| `sharky` | Diehl | Sharky 774/775 |
| `sensostar` | Engelmann | SensoStar |
| `qheat` | Qundis | Q heat |
| `compact5` | Kamstrup | MULTICAL 302/403 |
| `ultraheat` | Landis+Gyr | Ultraheat |
| `hydrocalm3` | Diehl | Hydrocalm 3 |

### Contadores Eléctricos

| Driver | Fabricante | Modelos |
|--------|------------|---------|
| `omnipower` | Kamstrup | OMNIPOWER |
| `amiplus` | Apator | Amiplus |
| `em24` | Carlo Gavazzi | EM24 |
| `iem3000` | Schneider Electric | iEM3000 |
| `ebzwmbe` | EBZ | WMB-E01 |

### Contadores de Gas

| Driver | Fabricante | Modelos |
|--------|------------|---------|
| `unismart` | Apator | Unismart |
| `bfw240radio` | BFW | 240 Radio |

### Repartidores de Costes de Calefacción

| Driver | Fabricante | Modelos |
|--------|------------|---------|
| `qcaloric` | Qundis | Q caloric |
| `fhkvdataiii` | Techem | FHKV data III |
| `fhkvdataiv` | Techem | FHKV data IV |
| `aventieshca` | Engelmann | Aventies |

### Detectores de Humo

| Driver | Fabricante | Modelos |
|--------|------------|---------|
| `ei6500` | Ei Electronics | Ei650 |
| `lansensm` | Lansen | Detectores de humo |

### Sensores de Temperatura/Humedad

| Driver | Fabricante | Modelos |
|--------|------------|---------|
| `rfmamb` | Diehl | RF Module Amb |
| `lansenth` | Lansen | Temperatura/Humedad |
| `munia` | Munia | Sensores T/H |

> 💡 **Consejo**: Configura `all_drivers: true` para cargar todos los drivers si no estás seguro de cuál usar. Revisa los logs para identificar el tipo de tu contador.

## Ejemplo Completo

```yaml
esphome:
  name: receptor-wmbus
  friendly_name: Receptor wM-Bus

esp32:
  board: esp32dev
  framework:
    type: esp-idf

logger:
  level: DEBUG

api:
  encryption:
    key: "tu-clave-api"

ota:
  platform: esphome

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

external_components:
  - source: github://jesusvallejo/components-esphome@main
    components: [ wmbus ]
    refresh: 0s

# Componente time requerido
time:
  - platform: homeassistant
    id: homeassistant_time

# Receptor wM-Bus
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
  log_all: true         # Habilitar para configuración inicial

# Contador de agua
sensor:
  - platform: wmbus
    meter_id: 0x12345678
    type: "multical21"
    key: "00112233445566778899AABBCCDDEEFF"  # Si está cifrado
    sensors:
      - name: "Consumo de Agua"
        field: "total"
        unit_of_measurement: "m³"
        accuracy_decimals: 3
        device_class: water
        state_class: total_increasing
        icon: "mdi:water"
        
      - name: "Agua Objetivo"
        field: "target"
        unit_of_measurement: "m³"
        accuracy_decimals: 3
        
      - name: "Caudal de Agua"
        field: "flow"
        unit_of_measurement: "m³/h"
        accuracy_decimals: 3
        icon: "mdi:water-pump"

# Contador de calor
  - platform: wmbus
    meter_id: 0xAABBCCDD
    type: "sharky"
    sensors:
      - name: "Energía Térmica"
        field: "total_energy"
        unit_of_measurement: "kWh"
        device_class: energy
        state_class: total_increasing
        
      - name: "Potencia Térmica"
        field: "power"
        unit_of_measurement: "kW"
        device_class: power

# Sensores de texto para estado
text_sensor:
  - platform: wmbus
    meter_id: 0x12345678
    type: "multical21"
    sensors:
      - name: "Estado Contador Agua"
        field: "status"
        icon: "mdi:information"
```

## Encontrar Información del Contador

### Paso 1: Habilitar Registro

```yaml
logger:
  level: DEBUG

wmbus:
  log_all: true
  all_drivers: true
```

### Paso 2: Revisar Logs

Cuando se recibe un telegrama, verás una salida como:

```
[D][wmbus:042]: Telegram received
[D][wmbus:043]: Meter ID: 12345678
[D][wmbus:044]: Type: multical21
[D][wmbus:045]: Manufacturer: KAM
[D][wmbus:046]: Fields: total=123.456 m³, target=120.000 m³
```

### Paso 3: Configurar Sensores

Usa el ID del contador, tipo y nombres de campos de los logs para configurar tus sensores.

## Claves de Cifrado

Muchos contadores usan cifrado AES. Para descifrar los datos:

1. **Contacta a tu proveedor de servicios** - Pueden proporcionarte la clave
2. **Revisa la documentación del contador** - Algunas claves están impresas en el contador
3. **Usa wmbusmeters en un PC** - Para identificar si se usa cifrado

### Formato de Clave

Las claves deben ser exactamente 32 caracteres hexadecimales (16 bytes):

```yaml
key: "00112233445566778899AABBCCDDEEFF"
```

### Sin Cifrado

Si el contador no está cifrado, deja la clave vacía u omítela:

```yaml
key: ""
# o simplemente no incluyas la opción key
```

## Solución de Problemas

### No se Reciben Telegramas

1. ✅ Comprueba el cableado SPI (MOSI, MISO, CLK, CS)
2. ✅ Verifica las conexiones GDO0 y GDO2
3. ✅ Asegúrate de que el CC1101 está a 3.3V (¡NO 5V!)
4. ✅ Verifica que la frecuencia coincide con tu región (868 MHz Europa, 915 MHz América)
5. ✅ Acércate al contador para pruebas
6. ✅ Habilita `log_all: true` para ver todos los telegramas recibidos

### Datos Incorrectos o Ausentes

1. ✅ Verifica que el ID del contador sea correcto (revisa los logs)
2. ✅ Verifica que el tipo/driver del contador sea correcto
3. ✅ Comprueba si se requiere clave de cifrado y si es correcta
4. ✅ Verifica que los nombres de campos coincidan con la salida del driver

### CC1101 No Encontrado

1. ✅ Revisa las conexiones SPI
2. ✅ Verifica que el pin CS esté correctamente configurado
3. ✅ Comprueba la alimentación (3.3V, corriente suficiente)
4. ✅ Prueba diferentes pines SPI

### Recepción Intermitente

1. ✅ Usa antena externa para mejor alcance
2. ✅ Reduce la distancia a los contadores
3. ✅ Comprueba si hay interferencias de radio
4. ✅ Asegura una fuente de alimentación estable

### Registro de Depuración

```yaml
logger:
  level: VERBOSE
  logs:
    wmbus: DEBUG
    cc1101: DEBUG
```

## Preguntas Frecuentes

**P: ¿Qué frecuencia debo usar?**
R: Europa usa 868.95 MHz, Norte/Sudamérica usa 915 MHz. Consulta las regulaciones locales.

**P: ¿Cómo encuentro el ID de mi contador?**
R: Habilita `log_all: true` y revisa los logs. El ID también suele estar impreso en el contador.

**P: ¿Puedo recibir múltiples contadores?**
R: ¡Sí! Añade múltiples configuraciones de sensores con diferentes valores de `meter_id`.

**P: ¿Cuál es el alcance máximo?**
R: Típicamente 50-100 metros en interiores, más con antena externa en línea de visión directa.

**P: ¿Necesito la clave de cifrado?**
R: Solo si tu contador transmite datos cifrados. Consulta con tu proveedor de servicios.

**P: ¿Por qué `all_drivers: true` tarda en compilar?**
R: Incluye los más de 80 drivers de contadores. Una vez identifiques tu tipo de contador, desactívalo y especifica solo el driver necesario.

**P: ¿Con qué frecuencia transmiten los contadores?**
R: Varía según el contador. Típicamente cada pocos segundos a cada pocos minutos.

**P: ¿Se soporta ESP8266?**
R: Solo ESP32 ha sido probado. ESP8266 podría funcionar pero no está soportado.

## Créditos

- Implementación original de wM-Bus fork de [@SzczepanLeon](https://github.com/SzczepanLeon)
- Integración CC1101 y mantenimiento por [@jesusvallejo](https://github.com/jesusvallejo)
- Basado en el proyecto [wmbusmeters](https://github.com/wmbusmeters/wmbusmeters)

## Licencia

Licencia MIT

Copyright (c) 2024-2026 jesusvallejo

Por la presente se concede permiso, libre de cargos, a cualquier persona que obtenga
una copia de este software y de los archivos de documentación asociados (el "Software"),
a utilizar el Software sin restricción, incluyendo sin limitación los derechos a usar,
copiar, modificar, fusionar, publicar, distribuir, sublicenciar, y/o vender copias del
Software, y a permitir a las personas a las que se les proporcione el Software a hacer
lo mismo, sujeto a las siguientes condiciones:

El aviso de copyright anterior y este aviso de permiso se incluirán en todas las copias
o partes sustanciales del Software.

EL SOFTWARE SE PROPORCIONA "COMO ESTÁ", SIN GARANTÍA DE NINGÚN TIPO, EXPRESA O
IMPLÍCITA, INCLUYENDO PERO NO LIMITADO A GARANTÍAS DE COMERCIALIZACIÓN, IDONEIDAD
PARA UN PROPÓSITO PARTICULAR E INCUMPLIMIENTO. EN NINGÚN CASO LOS AUTORES O
PROPIETARIOS DE LOS DERECHOS DE AUTOR SERÁN RESPONSABLES DE NINGUNA RECLAMACIÓN,
DAÑOS U OTRAS RESPONSABILIDADES, YA SEA EN UNA ACCIÓN DE CONTRATO, AGRAVIO O
CUALQUIER OTRO MOTIVO, DERIVADAS DE, FUERA DE O EN CONEXIÓN CON EL SOFTWARE O SU
USO U OTRO TIPO DE ACCIONES EN EL SOFTWARE.

## Autores

- **[@SzczepanLeon](https://github.com/SzczepanLeon)** - Implementación original de wM-Bus
- **[@jesusvallejo](https://github.com/jesusvallejo)** - Integración CC1101 y mantenedor del repositorio

## Agradecimientos

- Gracias a la comunidad de ESPHome
- Gracias al proyecto wmbusmeters por las implementaciones de drivers de contadores
- Gracias a todos los colaboradores y testers
