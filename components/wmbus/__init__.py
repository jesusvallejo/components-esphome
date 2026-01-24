import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import time
from esphome.const import (
    CONF_ID,
    CONF_MOSI_PIN,
    CONF_MISO_PIN,
    CONF_CLK_PIN,
    CONF_CS_PIN,
    CONF_TIME_ID,
    CONF_FREQUENCY,
)

CONF_GDO0_PIN = "gdo0_pin"
CONF_GDO2_PIN = "gdo2_pin"
CONF_LED_PIN = "led_pin"
CONF_LED_BLINK_TIME = "led_blink_time"
CONF_LOG_ALL = "log_all"
CONF_ALL_DRIVERS = "all_drivers"
CONF_SYNC_MODE = "sync_mode"
CONF_INFO_COMP_ID = "info_comp_id"

CODEOWNERS = ["@SzczepanLeon"]

DEPENDENCIES = ["time"]
AUTO_LOAD = ["sensor", "text_sensor"]

wmbus_ns = cg.esphome_ns.namespace('wmbus')
WMBusComponent = wmbus_ns.class_('WMBusComponent', cg.Component)
InfoComponent = wmbus_ns.class_('InfoComponent', cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_INFO_COMP_ID):                  cv.declare_id(InfoComponent),
    cv.GenerateID():                                   cv.declare_id(WMBusComponent),
    cv.OnlyWith(CONF_TIME_ID, "time"):                 cv.use_id(time.RealTimeClock),
    cv.Optional(CONF_MOSI_PIN,       default=13):      pins.internal_gpio_output_pin_schema,
    cv.Optional(CONF_MISO_PIN,       default=12):      pins.internal_gpio_input_pin_schema,
    cv.Optional(CONF_CLK_PIN,        default=14):      pins.internal_gpio_output_pin_schema,
    cv.Optional(CONF_CS_PIN,         default=2):       pins.internal_gpio_output_pin_schema,
    cv.Optional(CONF_GDO0_PIN,       default=5):       pins.internal_gpio_input_pin_schema,
    cv.Optional(CONF_GDO2_PIN,       default=4):       pins.internal_gpio_input_pin_schema,
    cv.Optional(CONF_LED_PIN):                         pins.gpio_output_pin_schema,
    cv.Optional(CONF_LED_BLINK_TIME, default="200ms"): cv.positive_time_period,
    cv.Optional(CONF_LOG_ALL,        default=False):   cv.boolean,
    cv.Optional(CONF_ALL_DRIVERS,    default=False):   cv.boolean,
    cv.Optional(CONF_FREQUENCY,      default=868.950): cv.float_range(min=300, max=928),
    cv.Optional(CONF_SYNC_MODE,      default=False):   cv.boolean,
})

async def to_code(config):
    var_adv = cg.new_Pvariable(config[CONF_INFO_COMP_ID])
    await cg.register_component(var_adv, {})

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    mosi = await cg.gpio_pin_expression(config[CONF_MOSI_PIN])
    miso = await cg.gpio_pin_expression(config[CONF_MISO_PIN])
    clk  = await cg.gpio_pin_expression(config[CONF_CLK_PIN])
    cs   = await cg.gpio_pin_expression(config[CONF_CS_PIN])
    gdo0 = await cg.gpio_pin_expression(config[CONF_GDO0_PIN])
    gdo2 = await cg.gpio_pin_expression(config[CONF_GDO2_PIN])

    cg.add(var.add_cc1101(mosi, miso, clk, cs, gdo0, gdo2, config[CONF_FREQUENCY], config[CONF_SYNC_MODE]))

    time_comp = await cg.get_variable(config[CONF_TIME_ID])
    cg.add(var.set_time(time_comp))

    cg.add(var.set_log_all(config[CONF_LOG_ALL]))

    if CONF_LED_PIN in config:
        led_pin = await cg.gpio_pin_expression(config[CONF_LED_PIN])
        cg.add(var.set_led_pin(led_pin))
        cg.add(var.set_led_blink_time(config[CONF_LED_BLINK_TIME].total_milliseconds))

    # No external libraries needed for ESP-IDF - using native SPI

    cg.add_platformio_option("build_src_filter", ["+<*>", "-<.git/>", "-<.svn/>"])

    if config[CONF_ALL_DRIVERS]:
        cg.add_platformio_option("build_src_filter", ["+<**/wmbus/driver_*.cpp>"])
    else:
        cg.add_platformio_option("build_src_filter", ["-<**/wmbus/driver_*.cpp>"])

    cg.add_platformio_option("build_src_filter", ["+<**/wmbus/driver_unknown.cpp>"])
