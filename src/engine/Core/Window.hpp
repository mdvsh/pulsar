#pragma once

#include <SDL2/SDL.h>

#include <string>

namespace App {

class Window {
 public:
  struct Settings {
    std::string title;
    int width{1280};
    int height{720};
    bool is_hidden{false};
  };

  explicit Window(const Settings& settings);
  ~Window();

  Window(const Window&) = delete;
  Window(Window&&) = delete;
  Window& operator=(Window other) = delete;
  Window& operator=(Window&& other) = delete;

  [[nodiscard]] SDL_Window* get_native_window() const;
  [[nodiscard]] SDL_Renderer* get_native_renderer() const;

  [[nodiscard]] bool has_mouse_focus() const { return m_MouseFocus; }
  [[nodiscard]] bool has_keyboard_focus() const { return m_KeyboardFocus; }
  [[nodiscard]] bool is_fullscreen() const { return m_FullScreen; }
  [[nodiscard]] bool is_shown() const { return m_Shown; }
  [[nodiscard]] Uint32 get_id() const { return m_ID; }

  void handle_event(const SDL_WindowEvent& event);
  void set_id() {
    m_ID = SDL_GetWindowID(m_window);
  }

 private:
  SDL_Window* m_window{nullptr};
  SDL_Renderer* m_renderer{nullptr};

  Uint32 m_ID{0};
  bool m_MouseFocus{false};
  bool m_KeyboardFocus{false};
  bool m_FullScreen{false};
  bool m_Shown{false};

};

}  // namespace App
