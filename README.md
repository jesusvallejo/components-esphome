External Components:
  - Golmar Uno: incoming call detection, button and switch support(auto turn-off after 2 secs)
```
external_components:
  - source: github://jesusvallejo/components-esphome/components@main
    components: [ golmar_uno ]
    refresh: 0s
```
example yaml:
```
external_components:
  - source: github://jesusvallejo/components-esphome/components@main
    components: [ golmar_uno ]
    refresh: 0s
    
esphome:
  name: test
  friendly_name: test

esp32:
  board: esp32-c3-devkitm-1
  framework:
    type: esp-idf

# Enable logging
logger:

# UART Configuration for communication with Arduino
uart:
  - id: arduino_uart
    tx_pin: GPIO10
    rx_pin: GPIO4
    baud_rate: 2600
    data_bits: 8
    parity: EVEN
    stop_bits: 1

golmar_uno:
    intercom_id: 0x11
    concierge_id: 0x00

switch:
  - platform: golmar_uno
    name: door
    
# button:
#   - platform: golmar_uno
#     name: door

binary_sensor:
  - platform: golmar_uno
    name: call
```
  
