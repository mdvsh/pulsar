//
// Created by Madhav Shekhar Sharma on 4/16/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_UI_H_
#define PULSAR_SRC_ENGINE_CORE_UI_H_

#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>
#include <imgui.h>
#include <unordered_set>
#include "Core/Log.hpp"
#include "Core/Resources.hpp"

namespace App {

class UI {
 public:
  void renderUI();
  void drawEditorPane();
  void drawAssetsPane();
  void drawCompPropsPane();
  void drawNewCompPane();
  void drawToolbar();
  void drawCenterPane();
  void drawPlaybackControls();

  void onQuitEvent();

 private:
  bool m_show_landing_panel{true};
  bool m_show_debug_panel{false};
  bool m_show_demo_panel{false};

  size_t selected_scene_index{0};
  std::string selected_actor;
  std::string selected_scene;
  ImGuiID dockspace_id{};
  std::vector<std::string> scene_actors;
  std::string resources_path_str = Resources::game_path().generic_string();
  const std::filesystem::path resources_path = Resources::game_path();
};

}  // namespace App

#endif  // PULSAR_SRC_ENGINE_CORE_UI_H_
