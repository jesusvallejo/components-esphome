import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

CODEOWNERS = ["@jesusvallejo"]
DEPENDENCIES = ['uart']
MULTI_CONF = True

CONF_T740UNO_ID = "t740uno_id"


t740uno_ns = cg.esphome_ns.namespace('t740uno')
T740UNOComponent = t740uno_ns.class_('T740UNOComponent', cg.Component, uart.UARTDevice)

CONF_UART_ID = 'uart_id'
CONF_INCOMING_CALL = 'incoming_call'

CONFIG_SCHEMA = (cv.Schema({
    cv.GenerateID(): cv.declare_id(T740UNOComponent),
})
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

