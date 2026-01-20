import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

CODEOWNERS = ["@jesusvallejo"]
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["binary_sensor", "button", "switch", "lock"]
MULTI_CONF = True

CONF_GOLMAR_UNO_ID = "golmar_uno_id"
CONF_CONCIERGE_ID = "concierge_id"
CONF_INTERCOM_ID = "intercom_id"
CONF_CALL_ALERT_DURATION = "call_alert_duration"
CONF_UNLOCK_TIMEOUT = "unlock_timeout"
CONF_COMMAND_DELAY = "command_delay"

# Minimum delay between commands per protocol specification
MIN_COMMAND_DELAY_MS = 500

golmar_uno_ns = cg.esphome_ns.namespace("golmar_uno")
GolmarUnoComponent = golmar_uno_ns.class_(
    "GolmarUnoComponent", cg.Component, uart.UARTDevice
)


def validate_command_delay(value):
    """Validate that command delay is at least 500ms."""
    value = cv.positive_time_period_milliseconds(value)
    if value.total_milliseconds < MIN_COMMAND_DELAY_MS:
        raise cv.Invalid(
            f"Command delay must be at least {MIN_COMMAND_DELAY_MS}ms, got {value.total_milliseconds}ms"
        )
    return value


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(GolmarUnoComponent),
            cv.Required(CONF_INTERCOM_ID): cv.int_range(min=0x00, max=0xFF),
            cv.Optional(CONF_CONCIERGE_ID, default=0x00): cv.int_range(min=0x00, max=0xFF),
            cv.Optional(CONF_CALL_ALERT_DURATION, default="2s"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_UNLOCK_TIMEOUT, default="1s"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_COMMAND_DELAY, default="500ms"): validate_command_delay,
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
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
    cg.add(var.set_concierge_id(config[CONF_CONCIERGE_ID]))
    cg.add(var.set_call_alert_duration(config[CONF_CALL_ALERT_DURATION]))
    cg.add(var.set_unlock_timeout(config[CONF_UNLOCK_TIMEOUT]))
    cg.add(var.set_command_delay(config[CONF_COMMAND_DELAY]))
