import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation 
from esphome.components import uart
# from esphome.components import sensor

from esphome.const import (
    CONF_ID,
    CONF_UART_ID
)

CODEOWNERS = ["@jesusvallejo"]  # Replace with your name
DEPENDENCIES = ["uart"]

CONF_T740UNO_ID = "t740uno_id"
CONF_RING = "ring"

t740uno_ns = cg.esphome_ns.namespace("t740uno")


T740UNO = t740uno_ns.class_(
    "T740UNO", cg.Component, uart.UARTDevice
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(T740UNO),
        cv.GenerateID(CONF_UART_ID): cv.use_id(uart.UARTComponent),
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
