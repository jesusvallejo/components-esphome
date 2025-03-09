import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

CODEOWNERS = ["@jesusvallejo"]
DEPENDENCIES = ['uart']
MULTI_CONF = True

CONF_T740UNO_ID = "t740uno_id"


t740uno_ns = cg.esphome_ns.namespace('t740uno')
T740UNO = t740uno_ns.class_('T740UNOComponent', cg.Component, uart.UARTDevice)

CONF_UART_ID = 'uart_id'

CONFIG_SCHEMA = (cv.Schema({
    cv.GenerateID(): cv.declare_id(T740UNO),
})
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "t740uno",
    require_tx=True,
    require_rx=True,
    baud_rate=2100,
    parity="NONE",
    stop_bits=1,
)



async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

