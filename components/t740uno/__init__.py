import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

CODEOWNERS = ["@jesusvallejo"]
DEPENDENCIES = ['uart']
MULTI_CONF = True

CONF_T740UNO_ID = "t740uno_id"


t740uno_ns = cg.esphome_ns.namespace('t740uno')
T740UNO = t740uno_ns.class_('T740UNOcomponent', cg.Component, uart.UARTDevice)

CONF_UART_ID = 'uart_id'
CONF_INCOMING_CALL = 'incoming_call'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(T740UNO),
    cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent)
})

async def to_code(config):
    uart_component = await cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID], uart_component)
    await cg.register_component(var, config)

