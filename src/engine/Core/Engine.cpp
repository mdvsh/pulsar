//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#include "Engine.h"
#include <rapidjson/document.h>
#include "AudioHelper.h"
#include "CameraManager.h"
#include "EngineUtils.h"
#include "EventBus.h"
#include "InputManager.h"
#include "Helper.h"

void Engine::initialize() {
  const std::string game_config_path =
      (App::Resources::game_path() / "game.config").generic_string();
  rapidjson::Document game_config;
  EngineUtils::ReadJsonFile(game_config_path, game_config);

  Renderer& renderer = Renderer::getInstance();
  SceneManager& scene_manager = SceneManager::getInstance();

  renderer.initialize(game_config);
  scene_manager.initialize(game_config);
  CameraManager::initialize();
  AudioManager::initialize();
  AudioManager::start_intro_music(game_config);
  InputManager::InitKeyToScancodeMap();
  InputManager::Init();
}

// void Engine::run_game() {
//   m_game_thread = std::thread(&Engine::game_loop, this);
// }

void Engine::stop_game() {
  set_engine_off();
}

void Engine::run_game() {
  Renderer& renderer = Renderer::getInstance();
  SceneManager& scene_manager = SceneManager::getInstance();

  if (not engine_running) {
    engine_running = true;
   SDL_ShowWindow(renderer.get_game_window());
   SDL_RaiseWindow(renderer.get_game_window());
  }

  while (engine_running) {
    SDL_Event input_event{};
    while (SDL_PollEvent(&input_event)) {
      if (input_event.type == SDL_QUIT) {
        set_engine_off();
      }
      if (input_event.window.windowID == renderer.m_window->get_id()) {
        on_game_window_event(input_event.window);
        InputManager::ProcessEvent(input_event);
      }
    }
    SDL_SetRenderDrawColor(renderer.get_sdl_renderer(), 0, 0, 0, 255);
    SDL_RenderClear(renderer.get_sdl_renderer());

    scene_manager.update_scene_actors();

    if (SceneManager::latest_scene_change_request) {
      scene_manager.trigger_scene_change(
          *SceneManager::latest_scene_change_request);
      SceneManager::latest_scene_change_request.reset();
    }

    App::EventBus::ProcessPendingSubscriptions();
    App::EventBus::ProcessPendingUnsubscriptions();

    if (not is_game_over and not is_game_won)
      renderer.end_of_frame_render();

    InputManager::LateUpdate();

    scene_manager.StepPhysWorld();

    SDL_RenderPresent(renderer.get_sdl_renderer());
  }
}

void Engine::on_game_window_event(const SDL_WindowEvent& event) {

  switch (event.event) {
    case SDL_WINDOWEVENT_CLOSE:
      return set_engine_off();
    // case SDL_WINDOWEVENT_MINIMIZED:
    //   return on_minimize();
    // case SDL_WINDOWEVENT_SHOWN:
    //   return on_shown();
    default:
      // Do nothing otherwise
      return;
  }
}

void Engine::reset() {
  SceneManager::getInstance().reset();
  engine_running = false;
  is_game_over = false;
  is_game_won = false;
  scene_change_queue = std::queue<std::string>();

  const std::string game_config_path =
      (App::Resources::game_path() / "game.config").generic_string();
  rapidjson::Document game_config;
  EngineUtils::ReadJsonFile(game_config_path, game_config);

  SceneManager& scene_manager = SceneManager::getInstance();
  scene_manager.initialize(game_config);
}
