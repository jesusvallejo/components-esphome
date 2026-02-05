# ESPHome External Components

> **Language / Idioma:** [English](#english) | [Español](#español)

---

<a name="english"></a>
## English

### Description

This repository contains external components for [ESPHome](https://esphome.io/) that enable integration of various devices with Home Assistant. These components provide advanced functionality for smart home automation.

### Available Components

| Component | Description | Documentation |
|-----------|-------------|---------------|
| **[golmar_uno](components/golmar_uno/)** | Integration for Golmar UNO intercom systems (T720/T540) - call detection and door unlock | [README](components/golmar_uno/README.md) |
| **[wmbus](components/wmbus/)** | Wireless M-Bus (wM-Bus) receiver for smart meters using CC1101 radio module | [README](components/wmbus/README.md) |

### Installation

Add the external component to your ESPHome configuration:

```yaml
# Golmar UNO
external_components:
  - source: github://jesusvallejo/components-esphome@main
    components: [ golmar_uno ]
    refresh: 0s

# wM-Bus
external_components:
  - source: github://jesusvallejo/components-esphome@main
    components: [ wmbus ]
    refresh: 0s
```

### Requirements

- [ESPHome](https://esphome.io/) 2023.12.0 or later
- [Home Assistant](https://www.home-assistant.io/) (recommended for full functionality)
- ESP32 board (tested platform)

### Component Details

#### Golmar UNO

ESPHome component for Golmar UNO intercom systems. Features include:
- 📞 Incoming call detection
- 🔓 Door unlock control (Button, Switch, Lock entities)
- 🔌 UART communication protocol

**[Full Documentation →](components/golmar_uno/README.md)**

#### wM-Bus (Wireless M-Bus)

ESPHome component for receiving and decoding wM-Bus telegrams from smart meters. Features include:
- 📡 CC1101 radio module support
- 🔐 AES decryption support
- 📊 Support for 80+ meter types
- ⚡ Real-time meter reading

**[Full Documentation →](components/wmbus/README.md)**

### Authors & Credits

- **Golmar UNO Component**: [@jesusvallejo](https://github.com/jesusvallejo)
- **wM-Bus Component**: Fork from [@SzczepanLeon](https://github.com/SzczepanLeon), CC1101 integration by [@jesusvallejo](https://github.com/jesusvallejo)

### License

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

---

<a name="español"></a>
## Español

### Descripción

Este repositorio contiene componentes externos para [ESPHome](https://esphome.io/) que permiten la integración de varios dispositivos con Home Assistant. Estos componentes proporcionan funcionalidad avanzada para automatización del hogar inteligente.

### Componentes Disponibles

| Componente | Descripción | Documentación |
|------------|-------------|---------------|
| **[golmar_uno](components/golmar_uno/)** | Integración para sistemas de portero Golmar UNO (T720/T540) - detección de llamadas y apertura de puerta | [README](components/golmar_uno/README_ES.md) |
| **[wmbus](components/wmbus/)** | Receptor Wireless M-Bus (wM-Bus) para contadores inteligentes usando módulo de radio CC1101 | [README](components/wmbus/README_ES.md) |

### Instalación

Añade el componente externo a tu configuración de ESPHome:

```yaml
# Golmar UNO
external_components:
  - source: github://jesusvallejo/components-esphome@main
    components: [ golmar_uno ]
    refresh: 0s

# wM-Bus
external_components:
  - source: github://jesusvallejo/components-esphome@main
    components: [ wmbus ]
    refresh: 0s
```

### Requisitos

- [ESPHome](https://esphome.io/) 2023.12.0 o posterior
- [Home Assistant](https://www.home-assistant.io/) (recomendado para funcionalidad completa)
- Placa ESP32 (plataforma probada)

### Detalles de los Componentes

#### Golmar UNO

Componente ESPHome para sistemas de portero Golmar UNO. Características:
- 📞 Detección de llamadas entrantes
- 🔓 Control de apertura de puerta (entidades Botón, Interruptor, Cerradura)
- 🔌 Protocolo de comunicación UART

**[Documentación Completa →](components/golmar_uno/README_ES.md)**

#### wM-Bus (Wireless M-Bus)

Componente ESPHome para recibir y decodificar telegramas wM-Bus de contadores inteligentes. Características:
- 📡 Soporte para módulo de radio CC1101
- 🔐 Soporte de descifrado AES
- 📊 Soporte para más de 80 tipos de contadores
- ⚡ Lectura de contadores en tiempo real

**[Documentación Completa →](components/wmbus/README_ES.md)**

### Autores y Créditos

- **Componente Golmar UNO**: [@jesusvallejo](https://github.com/jesusvallejo)
- **Componente wM-Bus**: Fork de [@SzczepanLeon](https://github.com/SzczepanLeon), integración CC1101 por [@jesusvallejo](https://github.com/jesusvallejo)

### Licencia

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
