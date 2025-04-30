import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

MULTI_CONF = False  # Set to True if multiple instances are allowed

# Create the namespace for box2d-lite
picocalc_ns = cg.esphome_ns.namespace('picocalc')
CtxGraphics = picocalc_ns.class_('CtxGraphics', cg.Component)

# Define the configuration schema
CONFIG_SCHEMA = (
    cv.Schema({cv.GenerateID(): cv.declare_id(CtxGraphics)})
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)