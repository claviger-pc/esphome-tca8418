# components/tca8418/__init__.py

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome import pins
from esphome.const import CONF_ID

DEPENDENCIES = ["i2c"]

tca8418_ns = cg.esphome_ns.namespace("tca8418")
TCA8418Component = tca8418_ns.class_("TCA8418Component", cg.Component, i2c.I2CDevice)

CONF_INTERRUPT_PIN = "interrupt_pin"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(TCA8418Component),
        cv.Optional(CONF_INTERRUPT_PIN): pins.internal_gpio_input_pin_schema,
    }
).extend(i2c.i2c_device_schema(0x34))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    # Register as a normal component
    await cg.register_component(var, config)

    # Register as an I2C device (handles address, i2c_id, etc.)
    await i2c.register_i2c_device(var, config)

    # Optional interrupt pin
    if CONF_INTERRUPT_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_INTERRUPT_PIN])
        cg.add(var.set_interrupt_pin(pin))
