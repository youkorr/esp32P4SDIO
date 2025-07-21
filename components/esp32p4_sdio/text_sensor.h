#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace esp32p4_sdio {

class ESP32P4SDIOComponent;

class ESP32P4SDIOTextSensor : public text_sensor::TextSensor, public Component {
 public:
  void update() override;
  void set_parent(ESP32P4SDIOComponent *parent) { this->parent_ = parent; }
  void set_file_path(const std::string &path) { this->file_path_ = path; }

 protected:
  ESP32P4SDIOComponent *parent_{nullptr};
  std::string file_path_;
};

}  // namespace esp32p4_sdio
}  // namespace esphome
