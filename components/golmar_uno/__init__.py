import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart,binary_sensor,button
from esphome.const import CONF_ID

CODEOWNERS = ["@jesusvallejo"]
DEPENDENCIES = ['uart', 'binary_sensor', 'button']
MULTI_CONF = True

CONF_GOLMAR_UNO_ID = "golmar_uno_id"

golmar_uno_ns = cg.esphome_ns.namespace('golmar_uno')
GolmarUnoComponent = golmar_uno_ns.class_('golmar_uno_component', cg.Component, uart.UARTDevice)

CONF_CONCIERGE_ID = 'concierge_id'
CONF_INTERCOM_ID = 'intercom_id'

CONFIG_SCHEMA = (cv.Schema({
    cv.GenerateID(): cv.declare_id(GolmarUnoComponent),
    cv.Required(CONF_INTERCOM_ID): cv.int_range(0x00, 0xFF),
    cv.Optional(CONF_CONCIERGE_ID): cv.int_range(0x00, 0xFF),
})
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)

FINAL_VALIDATE_UART_SCHEMA = uart.final_validate_device_schema(
    "golmar_uno",
    require_tx=True,
    require_rx=True,
    baud_rate=2600,
    parity="EVEN",
    stop_bits=1,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    cg.add(var.set_intercom_id(config[CONF_INTERCOM_ID]))
    if CONF_CONCIERGE_ID in config:
        cg.add(var.set_concierge_id(config[CONF_CONCIERGE_ID]))
    else:
        cg.add(var.set_concierge_id(0x00))
