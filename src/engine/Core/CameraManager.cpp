//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#include "CameraManager.h"
#include "EngineUtils.h"

namespace App {
const int CameraManager::DEFAULT_CAMERA_WIDTH = 13;
const int CameraManager::DEFAULT_CAMERA_HEIGHT = 9;

float CameraManager::zoom_factor;
float CameraManager::cam_x_pos;
float CameraManager::cam_y_pos;
int CameraManager::half_width;
int CameraManager::half_height;
float CameraManager::cam_ease_factor;

void CameraManager::initialize() {
  half_height = DEFAULT_CAMERA_HEIGHT / 2;
  half_width = DEFAULT_CAMERA_WIDTH / 2;
  zoom_factor = 1.0f;
  cam_ease_factor = 1.0f;
}

void CameraManager::set_position(const float x, const float y) {
  cam_x_pos = x;
  cam_y_pos = y;
}

float CameraManager::get_position_x() {
  return cam_x_pos;
}

float CameraManager::get_position_y() {
  return cam_y_pos;
}

void CameraManager::set_zoom(float zf) {
  CameraManager::zoom_factor = zf;
}

float CameraManager::get_zoom() {
  return zoom_factor;
}

void CameraManager::update_zoom_factor(const rapidjson::Document& docu_src) {
  if (const auto parsed_zoom_factor =
          EngineUtils::LoadFloatFromJson(docu_src, "parsed_zoom_factor");
      parsed_zoom_factor != 0.0f) {
    CameraManager::zoom_factor = parsed_zoom_factor;
  }
  if (const auto parsed_cam_ease_factor =
          EngineUtils::LoadFloatFromJson(docu_src, "camera_ease_factor");
      parsed_cam_ease_factor != 0.0f) {
    CameraManager::cam_ease_factor = parsed_cam_ease_factor;
  }
}

void CameraManager::calculate_camera_pos(const float camera_offset_x,
                                         const float camera_offset_y) {
  cam_x_pos += (camera_offset_x - cam_x_pos) * cam_ease_factor;
  cam_y_pos += (camera_offset_y - cam_y_pos) * cam_ease_factor;
}

}  // namespace App