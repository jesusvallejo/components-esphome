import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation 
from esphome.components import uart
from esphome.components import sensor

from esphome.const import (
    CONF_ID,
    CONF_UART_ID,
    UNIT_METER,
    DEVICE_CLASS_DISTANCE, 
    STATE_CLASS_MEASUREMENT
)

CODEOWNERS = ["@jesusvallejo"]  # Replace with your name
DEPENDENCIES = ["uart"]

CONF_T740UNO_ID = "t740uno_id"
CONF_RING = "ring"

t740uno_ns = cg.esphome_ns.namespace("t740uno")


T740UNO = t740uno_ns.class_(
    "T740UNO", cg.Component, uart.UARTDevice, sensor.Sensor
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(T740UNO),
        cv.GenerateID(CONF_UART_ID): cv.use_id(uart.UARTComponent),
        cv.Optional(CONF_RING): sensor.sensor_schema(
            unit_of_measurement=UNIT_METER,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_DISTANCE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    if CONF_RING in config:
        sens = await sensor.new_sensor(config[CONF_RING])
        cg.add(var.set_ring_sensor(sens))
     


# async def t740uno_open(config, action_id, template_args, args):
#     parent = await cg.get_variable(config)
#     return cg.new_Pvariable(action_id, parent.open_action()); # Returning a Pvariable for the action
