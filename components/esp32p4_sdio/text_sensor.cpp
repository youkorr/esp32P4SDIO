#include "text_sensor.h"
#include "esp32p4_sdio.h"
#include "esphome/core/log.h"

namespace esphome {
namespace esp32p4_sdio {

static const char *const TAG = "esp32p4_sdio.text_sensor";

void ESP32P4SDIOTextSensor::update() {
  if (this->parent_ == nullptr) {
    ESP_LOGE(TAG, "Parent component not set");
    return;
  }

  std::string content;
  if (!this->file_path_.empty()) {
    // Lecture d'un fichier spécifique
    if (this->parent_->read_file(this->file_path_, content)) {
      // Limiter la taille pour éviter les problèmes de mémoire
      if (content.length() > 1000) {
        content = content.substr(0, 1000) + "...";
      }
      this->publish_state(content);
    } else {
      this->publish_state("Error reading file");
    }
  } else {
    // Informations générales de la carte
    content = this->parent_->get_card_info();
    this->publish_state(content);
  }
}

}  // namespace esp32p4_sdio
}  // namespace esphome
