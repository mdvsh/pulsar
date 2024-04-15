//
// Created by Madhav Shekhar Sharma on 2/5/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_RENDERER_H_
#define PULSAR_SRC_ENGINE_CORE_RENDERER_H_

#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_ttf/SDL_ttf.h>
#include "Core/Resources.hpp"

// clang-format off
#include "lua.hpp"
#include <LuaBridge/LuaBridge.h>
// clang-format on

#include <rapidjson/document.h>
#include <unordered_map>
#include <utility>

#include "Actor.h"
#include "EngineUtils.h"
#include "Helper.h"

#include <deque>

struct TextRenderRequest {
  std::string text;
  int size;
  std::string font_name;
  SDL_Color color;
  int x, y;
};

class Renderer {
  static std::unordered_map<std::string, SDL_Texture*> textures;
  Renderer(){};
  int WINDOW_WIDTH = 640;
  int WINDOW_HEIGHT = 360;

  const float UNIT_DIST = 100.0f;
  const float DEFAULT_ZOOM_FACTOR = 1.0f;
  float ZOOM_FACTOR = DEFAULT_ZOOM_FACTOR;
  float CAMERA_EASE_FACTOR = 1.0f;

  std::string HEALTH_ICON;

  std::vector<std::string> intro_images;
  std::vector<std::string> intro_texts;
  std::unordered_map<std::string, std::unordered_map<int, TTF_Font*>>
      font_cache;
  static std::vector<TextRenderRequest> text_render_requests;
  static std::deque<IMGRenderRequest> img_render_requests;

  bool showed_intro = false;
  // const std::string IMAGES_PATH = "resources/images/";
  const std::string IMAGES_PATH = (App::Resources::game_path() / "images").generic_string();

 public:
  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;

  static Renderer& getInstance() {
    static Renderer instance;
    return instance;
  }

  [[nodiscard]] int get_window_width() const { return WINDOW_WIDTH; }
  [[nodiscard]] int get_window_height() const { return WINDOW_HEIGHT; }

  const int DEFAULT_TEXT_SIZE = 16;
  const SDL_Color DEFAULT_TEXT_COLOR = {255, 255, 255, 255};

  void initialize(const rapidjson::Document& game_config);
  void cleanup();

  ~Renderer() {
    for (auto& texture : textures) {
      SDL_DestroyTexture(texture.second);
    }
    textures.clear();
  }

  [[nodiscard]] float get_zoom_factor() const { return ZOOM_FACTOR; }

  [[nodiscard]] float get_camera_ease_factor() const {
    return CAMERA_EASE_FACTOR;
  }

  SDL_Renderer* sdl_renderer = nullptr;
  [[nodiscard]] SDL_Renderer* get_sdl_renderer() const { return sdl_renderer; }
  void set_sdl_renderer(SDL_Renderer* renderer) { sdl_renderer = renderer; }

  SDL_Window* game_window = nullptr;
  [[nodiscard]] SDL_Window* get_game_window() const { return game_window; }
  void set_game_window(SDL_Window* window) { game_window = window; }
  size_t intro_screen_index = 0;
  [[nodiscard]] size_t get_intro_image_index() const {
    return intro_screen_index;
  }
  void increment_intro_screen_index() { intro_screen_index++; }

  void render_intro();

  bool loaded_intro_images = false;
  bool loaded_intro_texts = false;  // if true, we also have set the font.

  bool setup_HUD(const rapidjson::Document& game_config);
  void load_intro_images(const rapidjson::Document& game_config);
  void load_intro_texts(const rapidjson::Document& game_config);

  [[nodiscard]] size_t get_image_count() const { return intro_images.size(); }
  [[nodiscard]] size_t get_text_count() const { return intro_texts.size(); }
  void render_cached_image(const std::string& image_name);

  void render_text(const std::string& text, const int x, const int y);
  void render_dialogues(
      const std::vector<std::pair<std::string, size_t>>& dialogues);
  void render_intro_text(const std::string& text);
  void render_cached_actor(Actor& actor,
                           const glm::vec2 camera_pos,
                           const EngineUtils::MoveIntent& move_intent);
  void render_HUD(int health, int score);
  static SDL_RendererFlip get_renderer_flip(const bool horizontal_flip,
                                            const bool vertical_flip);
  void service_text_render_requests();
  void set_showed_intro(const std::optional<bool> status = std::nullopt) {
    if (status.has_value())
      showed_intro = status.value();
    else
      showed_intro = (intro_texts.empty() and intro_images.empty());
    // Make sure only empty args when initialize.
  }
  [[nodiscard]] bool get_showed_intro() const { return showed_intro; }
  void renderCameraBounds(const glm::vec2& cameraPos,
                          int windowWidth,
                          int windowHeight,
                          float zoomFactor);

  static void cache_texture(const std::string& image_name);
  [[nodiscard]] static SDL_Texture* get_or_create_texture(
      const std::string& image_name);

  [[nodiscard]] static std::pair<int, int> get_image_dimensions(
      const std::string& image_name);
  [[nodiscard]] float get_render_unit_dist() const {
    return static_cast<float>(UNIT_DIST);
  }

  // DEBUG MODE FUNCTIONS
  SDL_Rect get_actor_aabb(const Actor& actor, const glm::vec2 camera_pos);
  const int EPS = 50;

  void end_of_frame_render();

  // HW 7 lua specific exposition ?
  static void log_error(const std::string& actor_name,
                        const luabridge::LuaException& e);
  static void LuaRenderText(const std::string& text,
                            int x,
                            int y,
                            const std::string& font_name,
                            int font_size,
                            int r,
                            int g,
                            int b,
                            int a);
  static void LuaDrawUI(std::string image_name, float x, float y);
  static void LuaDrawUIEx(std::string image_name,
                          float x,
                          float y,
                          float r,
                          float g,
                          float b,
                          float a,
                          int render_order);
  static void LuaDraw(std::string image_name, float x, float y);
  static void LuaDrawEx(std::string image_name,
                        float x,
                        float y,
                        float rotation_degrees,
                        float scale_x,
                        float scale_y,
                        float pivot_x,
                        float pivot_y,
                        float r,
                        float g,
                        float b,
                        float a,
                        int render_order);
  static void LuaDrawPixel(float x,
                           float y,
                           float r,
                           float g,
                           float b,
                           float a);

  void render_scene_image(const IMGRenderRequest& request) const;
  void render_UI_image(const IMGRenderRequest& request) const;
  void render_pixel(const IMGRenderRequest& request) const;
};

#endif  // PULSAR_SRC_ENGINE_CORE_RENDERER_H_
