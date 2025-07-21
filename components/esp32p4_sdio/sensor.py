import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, UNIT_MEGABYTES, DEVICE_CLASS_DATA_SIZE
from . import esp32p4_sdio_ns, ESP32P4SDIOComponent

DEPENDENCIES = ["esp32p4_sdio"]

ESP32P4SDIOSensor = esp32p4_sdio_ns.class_(
    "ESP32P4SDIOSensor", sensor.Sensor, cg.Component
)

CONF_SD_CARD_ID = "sd_card_id"

CONFIG_SCHEMA = sensor.sensor_schema(
    ESP32P4SDIOSensor,
    unit_of_measurement=UNIT_MEGABYTES,
    device_class=DEVICE_CLASS_DATA_SIZE,
).extend({
    cv.GenerateID(CONF_SD_CARD_ID): cv.use_id(ESP32P4SDIOComponent),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    
    parent = await cg.get_variable(config[CONF_SD_CARD_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.add_sensor(var))

