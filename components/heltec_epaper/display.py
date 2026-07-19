from esphome import pins
import esphome.codegen as cg
from esphome.components import display, spi
import esphome.config_validation as cv
from esphome.const import (
    CONF_BUSY_PIN,
    CONF_DC_PIN,
    CONF_ID,
    CONF_LAMBDA,
    CONF_PAGES,
    CONF_RESET_PIN,
)

CONF_POWER_PIN = "power_pin"
CONF_POWER_ON_DELAY = "power_on_delay"
CONF_BUSY_TIMEOUT = "busy_timeout"
CONF_FULL_UPDATE_EVERY = "full_update_every"

DEPENDENCIES = ["spi"]
AUTO_LOAD = ["display"]

heltec_epaper_ns = cg.esphome_ns.namespace("heltec_epaper")
HeltecE0213A367 = heltec_epaper_ns.class_(
    "HeltecE0213A367",
    display.DisplayBuffer,
    spi.SPIDevice,
)

CONFIG_SCHEMA = cv.All(
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(HeltecE0213A367),
            cv.Required(CONF_DC_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_RESET_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_BUSY_PIN): pins.gpio_input_pin_schema,
            cv.Optional(CONF_POWER_PIN, default=45): pins.gpio_output_pin_schema,
            cv.Optional(CONF_POWER_ON_DELAY, default="50ms"):
                cv.positive_time_period_milliseconds,
            cv.Optional(CONF_BUSY_TIMEOUT, default="20s"):
                cv.positive_time_period_milliseconds,
            cv.Optional(CONF_FULL_UPDATE_EVERY, default=10):
                cv.int_range(min=1, max=1000),
        }
    )
    .extend(cv.polling_component_schema("never"))
    .extend(spi.spi_device_schema()),
    cv.has_at_most_one_key(CONF_PAGES, CONF_LAMBDA),
)

FINAL_VALIDATE_SCHEMA = spi.final_validate_device_schema(
    "heltec_epaper",
    require_miso=False,
    require_mosi=True,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    # DisplayBuffer already derives from PollingComponent in ESPHome 2026.x.
    await display.register_display(var, config)
    await spi.register_spi_device(var, config, write_only=True)

    dc_pin = await cg.gpio_pin_expression(config[CONF_DC_PIN])
    cg.add(var.set_dc_pin(dc_pin))

    reset_pin = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
    cg.add(var.set_reset_pin(reset_pin))

    busy_pin = await cg.gpio_pin_expression(config[CONF_BUSY_PIN])
    cg.add(var.set_busy_pin(busy_pin))

    power_pin = await cg.gpio_pin_expression(config[CONF_POWER_PIN])
    cg.add(var.set_power_pin(power_pin))

    cg.add(var.set_power_on_delay(config[CONF_POWER_ON_DELAY]))
    cg.add(var.set_busy_timeout(config[CONF_BUSY_TIMEOUT]))
    cg.add(var.set_full_update_every(config[CONF_FULL_UPDATE_EVERY]))

    if CONF_LAMBDA in config:
        writer = await cg.process_lambda(
            config[CONF_LAMBDA],
            [(display.DisplayRef, "it")],
            return_type=cg.void,
        )
        cg.add(var.set_writer(writer))
