//
// Created by Madhav Shekhar Sharma on 1/24/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_ENGINE_H_
#define PULSAR_SRC_ENGINE_CORE_ENGINE_H_

#include <queue>
#include <string>
#include "Core/Renderer.h"
#include "Core/SceneManager.h"

#include <rapidjson/document.h>

enum class GameState { INTRO = 1, SCENE = 0, GAME_OVER = 0 };

class Engine {
 public:
  void load_initial_settings();
  void run_game();

  struct CameraBounds {
    int x_min;
    int x_max;
    int y_min;
    int y_max;
  };

  int CAMERA_WIDTH = 13;
  int CAMERA_HEIGHT = 9;

  glm::vec2 camera_position = {0.0f, 0.0f};
  [[nodiscard]] glm::vec2 get_camera_pos() const { return camera_position; }
  void set_camera_pos(const glm::vec2& pos) { camera_position = pos; }

  float PLAYER_SPEED = 0.02f;

  // move to scene manager
  const std::string HEALTH_DOWN_FLAG = "health down";
  const std::string SCORE_UP_FLAG = "score up";
  const std::string WIN_FLAG = "you win";
  const std::string LOSE_FLAG = "game over";

  std::unordered_map<std::string, glm::vec2> cardinal_directions_map = {
      {"n", glm::vec2(0, -1)},
      {"e", glm::vec2(1, 0)},
      {"s", glm::vec2(0, 1)},
      {"w", glm::vec2(-1, 0)}};

  // might not need starting HW7, keep for now
  glm::vec2 move_direction = {0.0f, 0.0f};

  std::vector<glm::vec2> ordinal_direction_vectors = {
      glm::vec2(1, -1), glm::vec2(1, 1), glm::vec2(-1, 1), glm::vec2(-1, -1)};

  bool engine_running = true;
  void set_engine_off() {
    if (engine_running)
      engine_running = false;
  }

 private:
  bool is_game_over = false;
  bool is_game_won = false;

  std::queue<std::string> scene_change_queue;
};

#endif  // PULSAR_SRC_ENGINE_CORE_ENGINE_H_
