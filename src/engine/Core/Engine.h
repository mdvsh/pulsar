//
// Created by Madhav Shekhar Sharma on 1/24/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_ENGINE_H_
#define PULSAR_SRC_ENGINE_CORE_ENGINE_H_

#include <queue>
#include <string>
#include <thread>
#include "Core/Renderer.h"
#include "Core/SceneManager.h"
#include "Core/Resources.hpp"

#include <rapidjson/document.h>

class Engine {

  Engine() = default;

 public:
  void initialize();
  void run_game();
  void stop_game();

  void on_game_window_event(const SDL_WindowEvent& event);

  void set_engine_off() {
    if (engine_running) {
      engine_running = false;
      SDL_HideWindow(Renderer::getInstance().get_game_window());  
    }
  }

  Engine (Engine const&) = delete;
  Engine& operator= (Engine const&) = delete;
  Engine (Engine&&) = delete;
  Engine& operator= (Engine&&) = delete;

  static Engine& getInstance() {
    static Engine instance;
    return instance;
  }

 private:
  bool engine_running{false};
  bool is_game_over{false};
  bool is_game_won{false};

  std::thread m_game_thread;
  std::queue<std::string> scene_change_queue;
};

#endif  // PULSAR_SRC_ENGINE_CORE_ENGINE_H_
