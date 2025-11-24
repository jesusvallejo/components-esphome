import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, button, number
from esphome.const import (
    CONF_ID,
    ENTITY_CATEGORY_CONFIG,
)

CODEOWNERS = ["@jesusvallejo"]
DEPENDENCIES = ['uart', 'binary_sensor', 'button', 'number']
MULTI_CONF = True

CONF_GOLMAR_UNO_ID = "golmar_uno_id"

golmar_uno_ns = cg.esphome_ns.namespace('golmar_uno')
GolmarUnoComponent = golmar_uno_ns.class_('golmar_uno_component', cg.Component, uart.UARTDevice)
GolmarUnoNumber = golmar_uno_ns.class_('GolmarUnoNumber', number.Number, cg.Component)

CONF_CONCIERGE_ID = 'concierge_id'
CONF_INTERCOM_ID = 'intercom_id'
CONF_DETECT_INTERCOM_ID_BUTTON = 'detect_intercom_id_button'
CONF_INTERCOM_ID_NUMBER = 'intercom_id_number'

CONFIG_SCHEMA = (cv.Schema({
    cv.GenerateID(): cv.declare_id(GolmarUnoComponent),
    cv.Optional(CONF_INTERCOM_ID, default=0x01): cv.int_range(0x00, 0xFF),
    cv.Optional(CONF_CONCIERGE_ID): cv.int_range(0x00, 0xFF),
    cv.Optional(CONF_DETECT_INTERCOM_ID_BUTTON): button.button_schema(),
    cv.Optional(CONF_INTERCOM_ID_NUMBER): number.number_schema(GolmarUnoNumber).extend({
        cv.GenerateID(): cv.declare_id(GolmarUnoNumber),
        cv.Optional("entity_category", default="config"): cv.entity_category,
    }),
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
    if CONF_DETECT_INTERCOM_ID_BUTTON in config:
        btn = await button.new_button(config[CONF_DETECT_INTERCOM_ID_BUTTON])
        cg.add(var.set_detect_intercom_id_button(btn))
    if CONF_INTERCOM_ID_NUMBER in config:
        num = await number.new_number(config[CONF_INTERCOM_ID_NUMBER], min_value=0, max_value=255, step=1)
        await cg.register_component(num, config[CONF_INTERCOM_ID_NUMBER])
        cg.add(var.set_intercom_id_number(num))
        cg.add(num.set_parent(var))
