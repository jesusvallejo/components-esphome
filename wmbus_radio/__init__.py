from contextlib import suppress
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins, automation
from esphome.components import spi
from esphome.cpp_generator import LambdaExpression
from esphome.const import (
    CONF_FREQUENCY,
    CONF_ID,
    CONF_RESET_PIN,
    CONF_IRQ_PIN,
    CONF_TRIGGER_ID,
    CONF_FORMAT,
    CONF_DATA,
)
from pathlib import Path

CODEOWNERS = ["@SzczepanLeon", "@kubasaw"]

DEPENDENCIES = ["esp32", "spi"]

AUTO_LOAD = ["wmbus_common"]

MULTI_CONF = True

CONF_RADIO_ID = "radio_id"
CONF_ON_FRAME = "on_frame"
CONF_RADIO_TYPE = "radio_type"
CONF_MARK_AS_HANDLED = "mark_as_handled"
CONF_BITRATE = "bitrate"
CONF_DEVIATION = "deviation"
CONF_RX_BANDWIDTH = "rx_bandwidth"
CONF_CHANNEL_SPACING = "channel_spacing"

radio_ns = cg.esphome_ns.namespace("wmbus_radio")
RadioComponent = radio_ns.class_("Radio", cg.Component)
RadioTransceiver = radio_ns.class_(
    "RadioTransceiver", spi.SPIDevice, cg.Component)
Frame = radio_ns.class_("Frame")
FrameOutputFormat = Frame.enum("OutputFormat")
FramePtr = Frame.operator("ptr")
FrameTrigger = radio_ns.class_(
    "FrameTrigger", automation.Trigger.template(FramePtr))

TRANSCEIVER_NAMES = {
    r.stem.removeprefix("transceiver_").upper()
    for r in Path(__file__).parent.glob("transceiver_*.cpp")
    if r.is_file()
} | {"CC1101"}

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(RadioComponent),
            cv.GenerateID(CONF_RADIO_ID): cv.declare_id(RadioTransceiver),
            cv.Required(CONF_RADIO_TYPE): cv.one_of(*TRANSCEIVER_NAMES, upper=True),
            cv.Required(CONF_RESET_PIN): pins.internal_gpio_output_pin_schema,
            cv.Required(CONF_IRQ_PIN): pins.internal_gpio_input_pin_schema,
            cv.Optional(CONF_FREQUENCY): cv.All(
                cv.frequency, cv.float_range(min=300.0e6, max=928.0e6)
            ),
            cv.Optional(CONF_BITRATE): cv.int_range(min=1000, max=500000),
            cv.Optional(CONF_DEVIATION): cv.All(
                cv.frequency, cv.float_range(min=1500, max=381000)
            ),
            cv.Optional(CONF_RX_BANDWIDTH): cv.All(
                cv.frequency, cv.float_range(min=58000, max=812000)
            ),
            cv.Optional(CONF_CHANNEL_SPACING): cv.All(
                cv.frequency, cv.float_range(min=25000, max=405000)
            ),
            cv.Optional(CONF_ON_FRAME): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(FrameTrigger),
                    cv.Optional(CONF_MARK_AS_HANDLED, default=False): cv.boolean,
                }
            ),
        }
    )
    .extend(spi.spi_device_schema())
    .extend(cv.COMPONENT_SCHEMA)
)


def _validate_cc1101_options(config):
    cc1101_keys = {
        CONF_FREQUENCY,
        CONF_BITRATE,
        CONF_DEVIATION,
        CONF_RX_BANDWIDTH,
        CONF_CHANNEL_SPACING,
    }
    if config[CONF_RADIO_TYPE] != "CC1101":
        for key in cc1101_keys:
            if key in config:
                raise cv.Invalid(f"{key} is only valid for radio_type: CC1101")
    return config


CONFIG_SCHEMA = cv.All(CONFIG_SCHEMA, _validate_cc1101_options)


async def to_code(config):
    cg.add(cg.LineComment("WMBus RadioTransceiver"))

    config[CONF_RADIO_ID].type = radio_ns.class_(
        config[CONF_RADIO_TYPE], RadioTransceiver
    )
    radio_var = cg.new_Pvariable(config[CONF_RADIO_ID])

    reset_pin = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
    cg.add(radio_var.set_reset_pin(reset_pin))

    irq_pin = await cg.gpio_pin_expression(config[CONF_IRQ_PIN])
    cg.add(radio_var.set_irq_pin(irq_pin))

    await spi.register_spi_device(radio_var, config)
    await cg.register_component(radio_var, config)

    if config[CONF_RADIO_TYPE] == "CC1101":
        if CONF_FREQUENCY in config:
            cg.add(radio_var.set_frequency(config[CONF_FREQUENCY]))
        if CONF_BITRATE in config:
            cg.add(radio_var.set_bitrate(config[CONF_BITRATE]))
        if CONF_DEVIATION in config:
            cg.add(radio_var.set_deviation(config[CONF_DEVIATION]))
        if CONF_RX_BANDWIDTH in config:
            cg.add(radio_var.set_rx_bandwidth(config[CONF_RX_BANDWIDTH]))
        if CONF_CHANNEL_SPACING in config:
            cg.add(radio_var.set_channel_spacing(config[CONF_CHANNEL_SPACING]))

    cg.add(cg.LineComment("WMBus Component"))
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_radio(radio_var))

    await cg.register_component(var, config)

    for conf in config.get(CONF_ON_FRAME, []):
        trig = cg.new_Pvariable(
            conf[CONF_TRIGGER_ID], var, conf[CONF_MARK_AS_HANDLED])
        await automation.build_automation(
            trig,
            [(FramePtr, "frame")],
            conf,
        )