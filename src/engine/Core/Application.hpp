#pragma once

#include <SDL2/SDL.h>
#include <condition_variable>
#include <filesystem>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "Core/Window.hpp"
#include "Core/UI.h"

namespace App {
enum class ExitStatus : int { SUCCESS = 0, FAILURE = 1 };
enum class AppEvent : int { QUIT = -1, NOTIFY = 0, WARN = 1 };

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
  void draw_playback_controls();
  static std::vector<std::string> get_proj_scene_files();

  void set_UI(std::unique_ptr<UI> ui) {
    m_ui = std::move(ui);
  }

  std::vector<std::string> scene_files;
  std::string selected_actor;
  std::string selected_scene;

 private:
  ExitStatus m_exit_status{ExitStatus::SUCCESS};
  std::unique_ptr<Window> m_window{nullptr};
  std::unique_ptr<UI> m_ui{nullptr};

  bool m_running{true};
  bool m_minimized{false};

  AppEvent m_app_event{AppEvent::NOTIFY};

  std::vector<std::string> actor_templates;
};

}  // namespace App
