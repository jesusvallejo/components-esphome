import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation, pins
from esphome.components import uart 
from esphome.components import binary_sensor
from esphome.automation import maybe_simple_id
from esphome.const import (
    CONF_ID,
    CONF_UART_ID,
    CONF_BINARY_SENSOR,
    CONF_NAME,
    CONF_DEVICE_CLASS,
    CONF_PLATFORM,
)

CODEOWNERS = ["@jesusvallejo"]  # Replace with your name
DEPENDENCIES = ["uart", "binary_sensor"]
MULTI_CONF = True

CONF_T740UNO_ID = "t740uno_id"
CONF_RING_BINARY_SENSOR = "ring"

T740UNO_OPEN_ACTION = "open"

t740uno_ns = cg.esphome_ns.namespace("t740uno")
T740UNOComponent = t740uno_ns.class_(
    "T740UNOComponent", cg.Component, uart.UARTDevice, cg.Controller
)
T740UNOComponentBlueprint = T740UNOComponent.new_blueprint

T740UNORingBinarySensor = t740uno_ns.class_(
    "T740UNORingBinarySensor", binary_sensor.BinarySensor
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(T740UNOComponentBlueprint),
        cv.GenerateID(CONF_UART_ID): cv.use_id(uart.UARTComponent),
        cv.Optional(CONF_RING_BINARY_SENSOR): cv.Optional({
            cv.GenerateID(): cv.declare_id(T740UNORingBinarySensor),
            cv.Required(CONF_NAME): cv.string,
            cv.Optional(CONF_DEVICE_CLASS): cv.Maybe(cv.string),
        }),
    }
).extend(uart.UART_DEVICE_SCHEMA)


async def to_code(config):
    parent = cg.get_variable(config[CONF_UART_ID])

    comp = cg.ComponentFactory.register(
        T740UNOComponentBlueprint,
        T740UNOComponentBlueprint(parent),
        config,
    )

    var = cg.new_Pvariable(config[CONF_T740UNO_ID], parent)
    await cg.register_component(var, config)

    # Ring Binary Sensor Configuration
    if CONF_RING_BINARY_SENSOR in config:
        sensor_conf = config[CONF_RING_BINARY_SENSOR]
        sens = cg.new_Pvariable(sensor_conf[CONF_ID], parent)
        cg.add(sens.set_parent(var))
        await binary_sensor.register_binary_sensor(sens, sensor_conf)
        if CONF_DEVICE_CLASS in sensor_conf:
            cg.add(sens.set_device_class(sensor_conf[CONF_DEVICE_CLASS]))
        cg.add(var.set_ring_sensor(sens))


    log_config = logger.get_logger(config)
    cg.add(var.set_logger(log_config))

    # Register component variable globally for C++ access
    cg.register_component_internal(var, "t740uno")

# Action code - service name shortened to open
@cg.esphome_ns.struct_("T740UNOOpenAction", cg.Action.template())
class T740UNOOpenAction(cg.Action):
    def __init__(self, parent):
        self.parent = parent

    def _parameters(self):
        return []
    def _has_async(self):
        return False
    @property
    def children(self):
        return []
    def to_code(self, parent, template_args, args, block):
        parent_ = cg.get_variable(self.parent)
        return cg.Return(parent_.open_action())


@T740UNOComponentBlueprint.action(
    T740UNO_OPEN_ACTION,
    return_type=T740UNOOpenAction,
)
async def t740uno_open_action_to_code(config, action_id, template_args, args):
    parent = await cg.get_variable(config)
    return T740UNOOpenAction(parent)
