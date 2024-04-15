#pragma once

#include <SDL2/SDL.h>

#include <memory>
#include <filesystem>
#include <queue>
#include <string>
#include <vector>

#include "Core/Window.hpp"

namespace App {

enum class ExitStatus : int { SUCCESS = 0, FAILURE = 1 };

class Application {
 public:
  explicit Application(const std::string& title);
  ~Application();

  Application(const Application&) = delete;
  Application(Application&&) = delete;
  Application& operator=(Application other) = delete;
  Application& operator=(Application&& other) = delete;

  ExitStatus run();
  void stop();

  void on_event(const SDL_WindowEvent& event);
  void on_minimize();
  void on_shown();
  void on_close();

  void open_project();
  void draw_editor_windows();
  static std::vector<std::string> get_proj_scene_files();

  bool engine_running = true;
  void set_engine_off() {
    if (engine_running)
      engine_running = false;
  }

  std::vector<std::string> scene_files;
  std::string selected_actor;
  std::string selected_scene;

 private:
  ExitStatus m_exit_status{ExitStatus::SUCCESS};
  std::unique_ptr<Window> m_window{nullptr};
  // std::unique_ptr<Engine>(m_engine){nullptr};

  bool m_running{true};
  bool m_minimized{false};
  bool m_show_landing_panel{true};
  bool m_show_debug_panel{false};
  bool m_show_demo_panel{false};
//  bool is_game_over{false};
//  bool is_game_won{false};

  std::queue<std::string> scene_change_queue;
};

}  // namespace App
