//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#include "Engine.h"

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

  rapidjson::Document game_config_document;
  EngineUtils::ReadJsonFile(game_config_path, game_config_document);

  game_state_messages.reserve(3);
  game_state_messages.push_back(EngineUtils::LoadStringFromJson(
      game_config_document, "game_start_message"));
  game_state_messages.push_back(EngineUtils::LoadStringFromJson(
      game_config_document, "game_over_bad_message"));
  game_state_messages.push_back(EngineUtils::LoadStringFromJson(
      game_config_document, "game_over_good_message"));

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
  std::cout << "Welcome to the game!" << '\n';
}
