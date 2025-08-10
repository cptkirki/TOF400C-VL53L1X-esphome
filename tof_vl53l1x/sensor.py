# /config/my_components/tof_vl53l1x/sensor.py
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID, UNIT_MILLIMETER, ICON_ARROW_EXPAND_VERTICAL,
    DEVICE_CLASS_DISTANCE, STATE_CLASS_MEASUREMENT
)

DEPENDENCIES = ["i2c"]

ns = cg.esphome_ns.namespace("tof_vl53l1x")
MyCustomSensor = ns.class_("MyCustomSensor", cg.PollingComponent, sensor.Sensor)

CONF_DISTANCE_MODE = "distance_mode"
CONF_TIMING_BUDGET_MS = "timing_budget_ms"
CONF_INTERMEASUREMENT_MS = "intermeasurement_ms"

DISTANCE_MODE = cv.one_of("short", "medium", "long", lower=True)

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        MyCustomSensor,
        unit_of_measurement=UNIT_MILLIMETER,
        icon=ICON_ARROW_EXPAND_VERTICAL,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_DISTANCE,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(cv.polling_component_schema("200ms"))
    .extend({
        cv.GenerateID(): cv.declare_id(MyCustomSensor),
        cv.Optional(CONF_DISTANCE_MODE, default="long"): DISTANCE_MODE,
        cv.Optional(CONF_TIMING_BUDGET_MS, default=50000): cv.positive_int,      # 50 ms
        cv.Optional(CONF_INTERMEASUREMENT_MS, default=200): cv.positive_int,     # 200 ms
    })
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)

    # distance_mode -> 0/1/2
    dm = config[CONF_DISTANCE_MODE]
    dm_val = 2 if dm == "long" else (1 if dm == "medium" else 0)
    cg.add(var.set_distance_mode(dm_val))

    cg.add(var.set_timing_budget_ms(config[CONF_TIMING_BUDGET_MS]))
    cg.add(var.set_intermeasurement_ms(config[CONF_INTERMEASUREMENT_MS]))
