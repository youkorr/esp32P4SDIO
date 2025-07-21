#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace esp32p4_sdio {

class ESP32P4SDIOComponent;

class ESP32P4SDIOSensor : public sensor::Sensor, public Component {
 public:
  void update() override;
  void set_parent(ESP32P4SDIOComponent *parent) { this->parent_ = parent; }

 protected:
  ESP32P4SDIOComponent *parent_{nullptr};
};

}  // namespace esp32p4_sdio
}  // namespace esphome
