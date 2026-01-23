# WMBus Radio (CC1101)

This component includes a CC1101 transceiver implementation that reads the raw stream for Wireless M-Bus.

## Example configuration

```yaml
wmbus_radio:
  id: wmbus_radio_1
  radio_id: wmbus_cc1101
  radio_type: CC1101
  reset_pin: GPIO5
  irq_pin: GPIO4  # CC1101 GDO0
  spi_id: spi_bus
  frequency: 868.95MHz
  bitrate: 100000
  deviation: 50kHz
  rx_bandwidth: 200kHz
  channel_spacing: 200kHz

spi:
  id: spi_bus
  clk_pin: GPIO18
  mosi_pin: GPIO23
  miso_pin: GPIO19
```

## Wiring notes

- Connect the CC1101 `GDO0` pin to the `irq_pin` configured above.
- Use a CC1101 module with a 26 MHz crystal (required for the configured RF settings).
- The default profile targets 868.95 MHz, 100 kbps, 2-FSK with 50 kHz deviation.
