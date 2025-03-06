import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, binary_sensor
from esphome.const import CONF_ID, DEVICE_CLASS_DOORBELL

DEPENDENCIES = ['uart']

t740uno_ns = cg.esphome_ns.namespace('t740uno')
T740Uno = t740uno_ns.class_('T740Uno', cg.Component, uart.UARTDevice)

CONF_UART_ID = 'uart_id'
CONF_INCOMING_CALL = 'incoming_call'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(T740Uno),
    cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
    cv.Required(CONF_INCOMING_CALL): binary_sensor.binary_sensor_schema(device_class=DEVICE_CLASS_DOORBELL),
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    uart_component = yield cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID], uart_component)
    yield cg.register_component(var, config)

    incoming_call = yield binary_sensor.new_binary_sensor(config[CONF_INCOMING_CALL])
    cg.add(var.set_incoming_call(incoming_call))
