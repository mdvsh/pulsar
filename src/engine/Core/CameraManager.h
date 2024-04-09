//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_CAMERAMANAGER_H_
#define PULSAR_SRC_ENGINE_CORE_CAMERAMANAGER_H_

#include <rapidjson/document.h>

namespace App {

struct CameraManager {
  static float zoom_factor;
  static float cam_x_pos, cam_y_pos;
  [[maybe_unused]] [[maybe_unused]] static int half_width, half_height;
  static float cam_ease_factor;

  static const int DEFAULT_CAMERA_WIDTH;
  static const int DEFAULT_CAMERA_HEIGHT;

  static void initialize();
  static void calculate_camera_pos(float camera_offset_x,
                                   float camera_offset_y);
  static void update_zoom_factor(const rapidjson::Document& docu_src);
  static void set_position(float x, float y);
  static float get_position_x();
  static float get_position_y();
  static void set_zoom(float zf);
  static float get_zoom();
};

}  // namespace App

#endif  // PULSAR_SRC_ENGINE_CORE_CAMERAMANAGER_H_
