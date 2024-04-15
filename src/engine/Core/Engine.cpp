//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#include "Engine.h"
#include "AudioHelper.h"
#include "CameraManager.h"
#include "EngineUtils.h"
#include "EventBus.h"
#include "InputManager.h"
#include <rapidjson/document.h>

void Engine::load_initial_settings() {
  const std::string resources_path = "resources/";
  const std::string game_config_path = resources_path + "game.config";
  const std::string rendering_config_path = resources_path + "rendering.config";

  if (!std::filesystem::exists(resources_path)) {
    std::cout << "error: resources/ missing";
    std::exit(0);
  }
  if (!std::filesystem::exists(game_config_path)) {
    std::cout << "error: resources/game.config missing";
    std::exit(0);
  }

  if (std::filesystem::exists(rendering_config_path)) {
    rapidjson::Document rendering_config_document;
    EngineUtils::ReadJsonFile(rendering_config_path, rendering_config_document);
    if (const int camera_width = EngineUtils::LoadIntFromJson(
            rendering_config_document, "x_resolution");
        camera_width != 0) {
      CAMERA_WIDTH = camera_width;
    }
    if (const int camera_height = EngineUtils::LoadIntFromJson(
            rendering_config_document, "y_resolution");
        camera_height != 0) {
      CAMERA_HEIGHT = camera_height;
    }
  }
}

void Engine::run_game() {
  const std::string game_config_path = "resources/game.config";
  rapidjson::Document game_config;
  EngineUtils::ReadJsonFile(game_config_path, game_config);
  
  Renderer& renderer = Renderer::getInstance();
  SceneManager& scene_manager = SceneManager::getInstance();

  renderer.initialize(game_config);
  CameraManager::initialize();

  scene_manager.initialize(game_config);

  AudioManager::initialize();
  AudioManager::start_intro_music(game_config);
  InputManager::InitKeyToScancodeMap();

  renderer.load_intro_images(game_config);
  renderer.load_intro_texts(game_config);

  renderer.set_showed_intro();
  InputManager::Init();
  while (engine_running) {
    SDL_Event input_event;
    while (Helper::SDL_PollEvent498(&input_event)) {
      InputManager::ProcessEvent(input_event);
      if (input_event.type == SDL_QUIT) {
        set_engine_off();
        // continue; ?
      }
      if (not renderer.get_showed_intro()) {
        // not showed intro input parsing
        if (input_event.type == SDL_MOUSEBUTTONDOWN &&
            input_event.button.button == SDL_BUTTON_LEFT) {
          renderer.increment_intro_screen_index();
          if (const size_t current_index = renderer.get_intro_image_index();
              current_index >= renderer.get_image_count() and
              current_index >= renderer.get_text_count()) {
            renderer.set_showed_intro(std::optional<bool>(true));
          }
          continue;
        }
        if (InputManager::GetKeyDown("space") ||
            InputManager::GetKeyDown("return")) {
          renderer.increment_intro_screen_index();
          if (const size_t current_index = renderer.get_intro_image_index();
              current_index >= renderer.get_image_count() &&
              current_index >= renderer.get_text_count()) {
            renderer.set_showed_intro(std::optional<bool>(true));
          }
        }
      }
    }
    SDL_RenderClear(renderer.get_sdl_renderer());
    if (not renderer.get_showed_intro()) {
      renderer.render_intro();
    } else {
      AudioManager::stop_intro_music();

      scene_manager.update_scene_actors();

      if (SceneManager::latest_scene_change_request) {
        scene_manager.trigger_scene_change(
            *SceneManager::latest_scene_change_request);
        SceneManager::latest_scene_change_request.reset();
      }
    }

    App::EventBus::ProcessPendingSubscriptions();
    App::EventBus::ProcessPendingUnsubscriptions();

    if (not is_game_over and not is_game_won)
      renderer.end_of_frame_render();

    InputManager::LateUpdate();

    scene_manager.StepPhysWorld();

    SDL_RenderPresent(renderer.get_sdl_renderer());
  }
  SDL_DestroyRenderer(renderer.get_sdl_renderer());
  SDL_DestroyWindow(renderer.get_game_window());
  SDL_Quit();
}
