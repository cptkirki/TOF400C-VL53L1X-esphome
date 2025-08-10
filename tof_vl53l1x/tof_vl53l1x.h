// /config/my_components/tof_vl53l1x/tof_vl53l1x.h
#pragma once
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include <Wire.h>
#include "VL53L1X.h"   // lokale Kopie (oder <VL53L1X.h>, falls per libraries: eingebunden)

namespace esphome {
namespace tof_vl53l1x {

class MyCustomSensor : public PollingComponent, public sensor::Sensor {
 public:
  MyCustomSensor() : PollingComponent(15000) {}

  // --- YAML-Setter ---
  void set_distance_mode(uint8_t mode) { distance_mode_ = mode; }             // 0=short,1=medium,2=long
  void set_timing_budget_ms(uint32_t ms) { timing_budget_ms_ = ms; }           // z.B. 50000
  void set_intermeasurement_ms(uint32_t ms) { intermeasurement_ms_ = ms; }     // z.B. 200

  void setup() override {
    // Wire.begin();  // ESPHome initialisiert I2C
    Wire.setClock(400000);

    tof_.setTimeout(500);
    if (!tof_.init()) {
      ESP_LOGE("tof_vl53l1x", "Failed to detect/init VL53L1X!");
      this->mark_failed();
      return;
    }

    // Distance-Mode anwenden
    switch (distance_mode_) {
      case 0: tof_.setDistanceMode(VL53L1X::Short); break;
      case 1: tof_.setDistanceMode(VL53L1X::Medium); break;
      default: tof_.setDistanceMode(VL53L1X::Long); break;
    }

    // Timing-Budget + Intermeasurement
    if (timing_budget_ms_ == 0) timing_budget_ms_ = 50000;
    if (intermeasurement_ms_ == 0) intermeasurement_ms_ = 200;

    tof_.setMeasurementTimingBudget(timing_budget_ms_);
    tof_.startContinuous(intermeasurement_ms_);

    ESP_LOGI("tof_vl53l1x", "Mode=%u, budget=%ums, period=%ums",
             distance_mode_, (unsigned)timing_budget_ms_, (unsigned)intermeasurement_ms_);
  }

  void update() override {
    uint16_t mm = tof_.read();
    if (!tof_.timeoutOccurred()) {
      this->publish_state(mm);
    } else {
      ESP_LOGW("tof_vl53l1x", "Timeout during read()");
    }
  }

 private:
    VL53L1X tof_;
    uint8_t  distance_mode_       = 2;      // default: long
    uint32_t timing_budget_ms_    = 50000;  // default: 50ms
    uint32_t intermeasurement_ms_ = 200;    // default: 200ms
};

}  // namespace tof_vl53l1x
}  // namespace esphome

