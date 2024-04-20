//
// Created by Madhav Shekhar Sharma on 2/5/24.
//

#include "Renderer.h"

#include "SDL2_image/SDL_image.h"
#include <filesystem>
#include "EngineUtils.h"

#include "Actor.h"
#include "CameraManager.h"
#include "Engine.h"

bool DEBUG_MODE = false;

std::vector<TextRenderRequest> Renderer::text_render_requests;
std::deque<IMGRenderRequest> Renderer::img_render_requests;
std::unordered_map<std::string, SDL_Texture*> Renderer::textures;

void Renderer::initialize(const rapidjson::Document& game_config) {
  const std::string game_title =
      EngineUtils::LoadStringFromJson(game_config, "game_title");
  // const std::string rendering_config_path = "resources/rendering.config";
  const std::string rendering_config_path = (App::Resources::game_path() / "rendering.config").generic_string();
  Uint8 clear_color_r = 255;
  Uint8 clear_color_g = 255;
  Uint8 clear_color_b = 255;
  if (std::filesystem::exists(rendering_config_path)) {
    rapidjson::Document rendering_config;
    EngineUtils::ReadJsonFile(rendering_config_path, rendering_config);

    if (const int width = EngineUtils::LoadIntFromJsonNegReturn(
            rendering_config, "x_resolution");
        width != -1)
      WINDOW_WIDTH = width;

    if (const int height = EngineUtils::LoadIntFromJsonNegReturn(
            rendering_config, "y_resolution");
        height != -1)
      WINDOW_HEIGHT = height;

    if (rendering_config.HasMember("clear_color_r"))
      clear_color_r =
          static_cast<Uint8>(rendering_config["clear_color_r"].GetInt());

    if (rendering_config.HasMember("clear_color_g"))
      clear_color_g =
          static_cast<Uint8>(rendering_config["clear_color_g"].GetInt());

    if (rendering_config.HasMember("clear_color_b"))
      clear_color_b =
          static_cast<Uint8>(rendering_config["clear_color_b"].GetInt());

    if (rendering_config.HasMember("zoom_factor"))
      ZOOM_FACTOR = rendering_config["zoom_factor"].GetFloat();

    if (rendering_config.HasMember("cam_ease_factor"))
      CAMERA_EASE_FACTOR = rendering_config["cam_ease_factor"].GetFloat();
  }

  const auto g_settings = App::Window::Settings{game_title, WINDOW_WIDTH, WINDOW_HEIGHT, false};
  m_window = std::make_unique<App::Window>(g_settings);
  m_window->set_id();
  
  SDL_SetRenderDrawColor(m_window->get_native_renderer(), clear_color_r, clear_color_g,
                         clear_color_b, 255);
  SDL_RenderClear(m_window->get_native_renderer());

  set_sdl_renderer(m_window->get_native_renderer());
  set_game_window(m_window->get_native_window());
}

void Renderer::render_HUD(const int health, const int score) {
  const auto [w, h] = get_image_dimensions(HEALTH_ICON);
  const int x = 5;
  const int y = 25;
  for (int i = 0; i < health; i++) {
    SDL_Rect render_rect = {x + i * (w + 5), y, w, h};
    SDL_RenderCopy(get_sdl_renderer(), textures[HEALTH_ICON], nullptr,
                   &render_rect);
  }
  render_text("score : " + std::to_string(score), 5, 5);
}

void Renderer::load_intro_images(const rapidjson::Document& game_config) {
  if (loaded_intro_images)
    return;
  if (game_config.HasMember("intro_image") &&
      game_config["intro_image"].IsArray()) {
    const auto& intro_images_array = game_config["intro_image"].GetArray();
    for (const auto& intro_image : intro_images_array) {
      const auto& name = intro_image.GetString();
      std::string img_path = "resources/images/" + std::string(name) + ".png";
      if (std::filesystem::exists(img_path)) {
        SDL_Texture* image_texture =
            IMG_LoadTexture(get_sdl_renderer(), img_path.c_str());
        textures[name] = image_texture;
        intro_images.emplace_back(name);
      } else {
        std::cout << "error: missing image " << name;
        std::exit(0);
      }
    }
    if (not loaded_intro_images)
      loaded_intro_images = true;
  }
}

void Renderer::load_intro_texts(const rapidjson::Document& game_config) {
  if (loaded_intro_texts)
    return;
  if (game_config.HasMember("font") && game_config["font"].IsString()) {
    const std::string font_path = "resources/fonts/" +
                                  std::string(game_config["font"].GetString()) +
                                  ".ttf";
    if (std::filesystem::exists(font_path)) {
      TTF_Font* font = TTF_OpenFont(font_path.c_str(), 16);
      font_cache[game_config["font"].GetString()][16] = font;
    } else {
      std::cout << "error: font " << game_config["font"].GetString()
                << " missing";
      std::exit(0);
    }
  }
  if (game_config.HasMember("intro_text") &&
      game_config["intro_text"].IsArray()) {
    const auto& intro_texts_array = game_config["intro_text"].GetArray();
    for (const auto& intro_text : intro_texts_array) {
      intro_texts.emplace_back(intro_text.GetString());
    }
    if (not intro_texts.empty() and not font_cache.empty()) {
      std::cout << "error: text render failed. No font configured";
      std::exit(0);
    }
    if (not loaded_intro_texts)
      loaded_intro_texts = true;
  }
}

void Renderer::render_cached_image(const std::string& image_name) {
  SDL_Texture* image_texture = textures[image_name];
  SDL_RenderCopy(get_sdl_renderer(), image_texture, nullptr, nullptr);
}

void Renderer::LuaRenderText(const std::string& text,
                             const int x,
                             const int y,
                             const std::string& font_name,
                             const int font_size,
                             const int r,
                             const int g,
                             const int b,
                             const int a) {
  text_render_requests.push_back(
      {text,
       font_size,
       font_name,
       {static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b),
        static_cast<Uint8>(a)},
       x,
       y});
}
void Renderer::LuaDrawUI(std::string image_name, float x, float y) {
  cache_texture(image_name);
  const IMGRenderRequest request{IMGType::UI, std::move(image_name), x, y};
  img_render_requests.push_back(request);
}
void Renderer::LuaDrawUIEx(std::string image_name,
                           float x,
                           float y,
                           float r,
                           float g,
                           float b,
                           float a,
                           int render_order) {
  cache_texture(image_name);
  IMGRenderRequest request{IMGType::UI, std::move(image_name), x, y,
                           render_order};
  request.r = static_cast<int>(std::floor(r));
  request.g = static_cast<int>(std::floor(g));
  request.b = static_cast<int>(std::floor(b));
  request.a = static_cast<int>(std::floor(a));
  img_render_requests.push_back(request);
}
void Renderer::LuaDraw(std::string image_name, float x, float y) {
  cache_texture(image_name);
  const IMGRenderRequest request{IMGType::Scene, std::move(image_name), x, y};
  img_render_requests.push_back(request);
}
void Renderer::LuaDrawEx(std::string image_name,
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
                         int render_order) {
  cache_texture(image_name);
  IMGRenderRequest request{IMGType::Scene, std::move(image_name), x, y,
                           render_order};
  request.rotation_degrees = static_cast<int>(rotation_degrees);
  request.scale_x = scale_x;
  request.scale_y = scale_y;
  request.pivot_x = pivot_x;
  request.pivot_y = pivot_y;
  request.r = static_cast<int>(std::floor(r));
  request.g = static_cast<int>(std::floor(g));
  request.b = static_cast<int>(std::floor(b));
  request.a = static_cast<int>(std::floor(a));
  img_render_requests.push_back(request);
}
void Renderer::LuaDrawPixel(float x,
                            float y,
                            float r,
                            float g,
                            float b,
                            float a) {
  IMGRenderRequest request{IMGType::Pixel, "", x, y};
  request.r = static_cast<int>(std::floor(r));
  request.g = static_cast<int>(std::floor(g));
  request.b = static_cast<int>(std::floor(b));
  request.a = static_cast<int>(std::floor(a));
  img_render_requests.push_back(request);
}
void Renderer::render_scene_image(const IMGRenderRequest& request) const {
  SDL_RenderSetScale(get_sdl_renderer(), CameraManager::zoom_factor,
                     CameraManager::zoom_factor);
  const auto texture = get_or_create_texture(request.image_name);
  SDL_Rect render_rect = {0, 0, 0, 0};
  SDL_QueryTexture(texture, nullptr, nullptr, &render_rect.w, &render_rect.h);
  const int w = render_rect.w;
  const int h = render_rect.h;
  float pivot_x, pivot_y;
  if (request.pivot_x == -1.0f)
    pivot_x = static_cast<int>(w * 0.5);
  else
    pivot_x = static_cast<int>(request.pivot_x * w * request.scale_x);
  if (request.pivot_y == -1.0f)
    pivot_y = static_cast<int>(h * 0.5);
  else
    pivot_y = static_cast<int>(request.pivot_y * h * request.scale_y);

  render_rect.x = static_cast<int>(
      WINDOW_WIDTH / 2 / CameraManager::zoom_factor +
      (request.x - CameraManager::cam_x_pos) * UNIT_DIST - pivot_x);
  render_rect.y = static_cast<int>(
      WINDOW_HEIGHT / 2 / CameraManager::zoom_factor +
      (request.y - CameraManager::cam_y_pos) * UNIT_DIST - pivot_y);
  render_rect.w = static_cast<int>(w * fabs(request.scale_x));
  render_rect.h = static_cast<int>(h * fabs(request.scale_y));

  SDL_SetTextureColorMod(texture, request.r, request.g, request.b);
  SDL_SetTextureAlphaMod(texture, request.a);
  const SDL_Point pivot = {static_cast<int>(pivot_x),
                           static_cast<int>(pivot_y)};
  SDL_RenderCopyEx(get_sdl_renderer(), texture, nullptr,
                              &render_rect, request.rotation_degrees, &pivot,
                              SDL_FLIP_NONE);
  SDL_RenderSetScale(get_sdl_renderer(), DEFAULT_ZOOM_FACTOR,
                     DEFAULT_ZOOM_FACTOR);
  SDL_SetTextureColorMod(texture, 255, 255, 255);
  SDL_SetTextureAlphaMod(texture, 255);
}
void Renderer::render_UI_image(const IMGRenderRequest& request) const {
  const auto texture = get_or_create_texture(request.image_name);
  SDL_Rect render_rect = {static_cast<int>(request.x),
                          static_cast<int>(request.y), 0, 0};
  SDL_QueryTexture(texture, nullptr, nullptr, &render_rect.w, &render_rect.h);
  SDL_SetTextureColorMod(texture, request.r, request.g, request.b);
  SDL_SetTextureAlphaMod(texture, request.a);
  SDL_RenderCopy(get_sdl_renderer(), texture, nullptr, &render_rect);
  SDL_SetTextureColorMod(texture, 255, 255, 255);
  SDL_SetTextureAlphaMod(texture, 255);
}
void Renderer::render_pixel(const IMGRenderRequest& request) const {
  SDL_SetRenderDrawColor(get_sdl_renderer(), request.r, request.g, request.b,
                         request.a);
  SDL_SetRenderDrawBlendMode(get_sdl_renderer(), SDL_BLENDMODE_BLEND);
  SDL_RenderDrawPoint(get_sdl_renderer(), static_cast<int>(request.x),
                      static_cast<int>(request.y));
  SDL_SetRenderDrawColor(get_sdl_renderer(), 255, 255, 255, 255);
  SDL_SetRenderDrawBlendMode(get_sdl_renderer(), SDL_BLENDMODE_NONE);
}

void Renderer::render_dialogues(
    const std::vector<std::pair<std::string, size_t> >& dialogues) {
  const size_t m = dialogues.size();
  for (size_t i = 0; i < m; i++) {
    render_text(dialogues[i].first, 25,
                static_cast<int>(WINDOW_HEIGHT - 50 - 50 * (m - 1 - i)));
  }
}

void Renderer::render_text(const std::string& text, const int x, const int y) {
  // arial for now wouldnt really work but helps make it backwards compatible
  text_render_requests.push_back(
      {text, DEFAULT_TEXT_SIZE, "arial", DEFAULT_TEXT_COLOR, x, y});
}

void Renderer::service_text_render_requests() {
  for (const auto& [text, size, font, color, x, y] : text_render_requests) {
    TTF_Font* ttf_font = nullptr;
    if (font_cache.count(font) > 0 and font_cache[font].count(size) > 0) {
      ttf_font = font_cache[font][size];
    } else {
      const std::string font_path = FONTS_PATH + font + ".ttf";
      if (std::filesystem::exists(font_path)) {
        ttf_font =
            TTF_OpenFont((font_path).c_str(), size);
        font_cache[font][size] = ttf_font;
      } else {
        std::cout << "error: font " << font << " missing";
        std::exit(0);
      }
    }
    SDL_Surface* surface = TTF_RenderText_Solid(ttf_font, text.c_str(), color);
    SDL_Texture* texture =
        SDL_CreateTextureFromSurface(get_sdl_renderer(), surface);
    SDL_Rect text_rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(get_sdl_renderer(), texture, nullptr, &text_rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
  }
  text_render_requests.clear();
}

void Renderer::end_of_frame_render() {
  std::stable_sort(img_render_requests.begin(), img_render_requests.end(),
                   [](const IMGRenderRequest& a, const IMGRenderRequest& b) {
                     if (a.type != b.type)
                       return a.type < b.type;
                     return a.render_order < b.render_order;
                   });
  for (const auto& request : img_render_requests) {
    switch (request.type) {
      case IMGType::Scene:
        render_scene_image(request);
        break;
      case IMGType::UI:
        render_UI_image(request);
        break;
      case IMGType::Pixel:
        if (not text_render_requests.empty())
          service_text_render_requests();
        render_pixel(request);
        break;
    }
  }
  if (not text_render_requests.empty())
    service_text_render_requests();
  img_render_requests.clear();
}

void Renderer::cache_texture(const std::string& image_name) {
  if (textures.count(image_name) > 0)
    return;
  const auto& renderer = getInstance();
  const std::string image_path = renderer.IMAGES_PATH + image_name + ".png";
  if (std::filesystem::exists(image_path)) {
    SDL_Surface* image_surface = IMG_Load(image_path.c_str());
    SDL_Texture *image_texture = SDL_CreateTextureFromSurface(renderer.get_sdl_renderer(), image_surface);
    if (image_texture == nullptr) {
      std::cerr << "Error loading texture " << image_name << ": " << IMG_GetError() << std::endl;
    } else {
      textures[image_name] = image_texture;
    }
  } else {
    std::cout << "error: missing image " << image_name;
    std::exit(0);
  }
}

SDL_Texture* Renderer::get_or_create_texture(const std::string& image_name) {
  if (textures.count(image_name) > 0)
    return textures.at(image_name);
  cache_texture(image_name);
  //    std::cout << "[SUS] SPRITE " << image_name << " NOT PREVIOUSLY
  //    CACHED!\n";
  return textures.at(image_name);
}

std::pair<int, int> Renderer::get_image_dimensions(
    const std::string& image_name) {
  SDL_Texture* image_texture = get_or_create_texture(image_name);
  int w, h;
  SDL_QueryTexture(image_texture, nullptr, nullptr, &w, &h);
  return {w, h};
}
void Renderer::log_error(const std::string& actor_name,
                         const luabridge::LuaException& e) {
  std::string error_message = e.what();
  /* Normalize file paths across platforms */
  std::replace(error_message.begin(), error_message.end(), '\\', '/');
  /* Display (with color codes) */
  std::cout << "\033[31m" << actor_name << " : " << error_message << "\033[0m"
            << std::endl;
}

void Renderer::render_intro_text(const std::string& text) {
  const int default_x = 25;
  const int default_y = WINDOW_HEIGHT - 50;
  render_text(text, default_x, default_y);
}

void Renderer::render_intro() {
  if (not intro_images.empty()) {
    if (intro_images.size() > intro_screen_index) {
      render_cached_image(intro_images[intro_screen_index]);
    } else {
      render_cached_image(intro_images.back());
    }
  }
  if (not intro_texts.empty()) {
    if (intro_texts.size() > intro_screen_index) {
      render_intro_text(intro_texts[intro_screen_index]);
    } else {
      render_intro_text(intro_texts.back());
    }
  }
}

SDL_RendererFlip Renderer::get_renderer_flip(const bool horizontal_flip,
                                             const bool vertical_flip) {
  int flip = 0;
  if (horizontal_flip) {
    flip |= SDL_FLIP_HORIZONTAL;
  }
  if (vertical_flip) {
    flip |= SDL_FLIP_VERTICAL;
  }
  return static_cast<SDL_RendererFlip>(flip);
}
