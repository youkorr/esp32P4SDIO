import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome import pins

DEPENDENCIES = ["esp32"]
CODEOWNERS = ["@youkorr"]

esp32p4_sdio_ns = cg.esphome_ns.namespace("esp32p4_sdio")
ESP32P4SDIOComponent = esp32p4_sdio_ns.class_("ESP32P4SDIOComponent", cg.Component)

CONF_DAT0_PIN = "dat0_pin"
CONF_DAT1_PIN = "dat1_pin"
CONF_DAT2_PIN = "dat2_pin"
CONF_DAT3_PIN = "dat3_pin"
CONF_CLK_PIN = "clk_pin"
CONF_CMD_PIN = "cmd_pin"
CONF_FREQUENCY = "frequency"
CONF_MOUNT_POINT = "mount_point"
CONF_SLOT = "slot"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ESP32P4SDIOComponent),
    cv.Required(CONF_DAT0_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_DAT1_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_DAT2_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_DAT3_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_CLK_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_CMD_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_FREQUENCY, default="20MHz"): cv.frequency,
    cv.Optional(CONF_MOUNT_POINT, default="/sdcard"): cv.string,
    cv.Optional(CONF_SLOT, default=1): cv.int_range(min=0, max=1),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    cg.add(var.set_pins(
        await cg.gpio_pin_expression(config[CONF_DAT0_PIN]),
        await cg.gpio_pin_expression(config[CONF_DAT1_PIN]),
        await cg.gpio_pin_expression(config[CONF_DAT2_PIN]),
        await cg.gpio_pin_expression(config[CONF_DAT3_PIN]),
        await cg.gpio_pin_expression(config[CONF_CLK_PIN]),
        await cg.gpio_pin_expression(config[CONF_CMD_PIN])
    ))
    cg.add(var.set_frequency(config[CONF_FREQUENCY]))
    cg.add(var.set_mount_point(config[CONF_MOUNT_POINT]))
    cg.add(var.set_slot(config[CONF_SLOT]))

