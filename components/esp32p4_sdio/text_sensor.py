import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID
from . import esp32p4_sdio_ns, ESP32P4SDIOComponent

DEPENDENCIES = ["esp32p4_sdio"]

ESP32P4SDIOTextSensor = esp32p4_sdio_ns.class_(
    "ESP32P4SDIOTextSensor", text_sensor.TextSensor, cg.Component
)

CONF_SD_CARD_ID = "sd_card_id"
CONF_FILE_PATH = "file_path"

CONFIG_SCHEMA = text_sensor.text_sensor_schema().extend({
    cv.GenerateID(): cv.declare_id(ESP32P4SDIOTextSensor),
    cv.GenerateID(CONF_SD_CARD_ID): cv.use_id(ESP32P4SDIOComponent),
    cv.Optional(CONF_FILE_PATH): cv.string,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = await text_sensor.new_text_sensor(config)
    await cg.register_component(var, config)
    
    parent = await cg.get_variable(config[CONF_SD_CARD_ID])
    cg.add(var.set_parent(parent))
    
    if CONF_FILE_PATH in config:
        cg.add(var.set_file_path(config[CONF_FILE_PATH]))
