#include "Application.hpp"

#include <SDL2/SDL.h>

// For clang-tidy include check
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_filesystem.h>
#include <SDL_render.h>
#include <SDL_video.h>

#include <rapidjson/document.h>

#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>
#include <imgui.h>

#include <memory>
#include <string>

#include "Core/AudioManager.h"
#include "Core/CameraManager.h"
#include "Core/DPIHandler.hpp"
#include "Core/Debug/Instrumentor.hpp"
#include "Core/EngineUtils.h"
#include "Core/EventBus.h"
#include "Core/Helper.h"
#include "Core/InputManager.h"
#include "Core/Log.hpp"
#include "Core/Renderer.h"
#include "Core/Resources.hpp"
#include "Core/SceneManager.h"
#include "Core/Window.hpp"
#include "Settings/Project.hpp"

namespace App {

Application::Application(const std::string& title) {
  APP_PROFILE_FUNCTION();

  const unsigned int init_flags{SDL_INIT_VIDEO | SDL_INIT_TIMER |
                                SDL_INIT_GAMECONTROLLER};
  if (SDL_Init(init_flags) != 0) {
    APP_ERROR("Error: %s\n", SDL_GetError());
    m_exit_status = ExitStatus::FAILURE;
  }

  m_window = std::make_unique<Window>(Window::Settings{title});
}

Application::~Application() {
  APP_PROFILE_FUNCTION();

  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_Quit();
}

ExitStatus App::Application::run() {
  APP_PROFILE_FUNCTION();

  if (m_exit_status == ExitStatus::FAILURE) {
    return m_exit_status;
  }

  // Load up

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io{ImGui::GetIO()};

  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard |
                    ImGuiConfigFlags_DockingEnable |
                    ImGuiConfigFlags_ViewportsEnable;

  const std::string user_config_path{
      SDL_GetPrefPath(COMPANY_NAMESPACE.c_str(), APP_NAME.c_str())};
  APP_DEBUG("User config path: {}", user_config_path);

  // Absolute imgui.ini path to preserve settings independent of app location.
  static const std::string imgui_ini_filename{user_config_path + "imgui.ini"};
  io.IniFilename = imgui_ini_filename.c_str();

  // ImGUI font
  const float font_scaling_factor{DPIHandler::get_scale()};
  const float font_size{18.0F * font_scaling_factor};
  const std::string font_path{
      Resources::font_path("Manrope.ttf").generic_string()};
  io.Fonts->AddFontFromFileTTF(font_path.c_str(), font_size);
  io.FontDefault = io.Fonts->AddFontFromFileTTF(font_path.c_str(), font_size);
  DPIHandler::set_global_font_scaling(&io);

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForSDLRenderer(m_window->get_native_window(),
                                    m_window->get_native_renderer());
  ImGui_ImplSDLRenderer2_Init(m_window->get_native_renderer());

  scene_files = get_proj_scene_files();
  size_t selected_scene_index = 0;

  const std::string game_config_path =
      (Resources::game_path() / "game.config").generic_string();
  rapidjson::Document game_config;
  EngineUtils::ReadJsonFile(game_config_path, game_config);
  SceneManager& scene_manager = SceneManager::getInstance();
  scene_manager.initialize(game_config);

  m_running = true;
  while (m_running) {
    APP_PROFILE_SCOPE("MainLoop");

    SDL_Event event{};
    while (SDL_PollEvent(&event) == 1) {
      APP_PROFILE_SCOPE("EventPolling");

      ImGui_ImplSDL2_ProcessEvent(&event);

      if (event.type == SDL_QUIT) {
        stop();
      }

      if (event.type == SDL_WINDOWEVENT &&
          event.window.windowID ==
              SDL_GetWindowID(m_window->get_native_window())) {
        on_event(event.window);
      }
    }

    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (!m_minimized) {
      ImGui::DockSpaceOverViewport();
      ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
      ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f),
                       ImGuiDockNodeFlags_PassthruCentralNode);

      if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
          if (ImGui::MenuItem("Open Project", "Cmd+O")) {
            open_project();
          }
          if (ImGui::MenuItem("Exit", "Cmd+Q")) {
            stop();
          }
          ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
          ImGui::MenuItem("Pulsar", nullptr, &m_show_landing_panel);
          ImGui::MenuItem("ImGui Demo Panel", nullptr, &m_show_demo_panel);
          ImGui::MenuItem("Debug Panel", nullptr, &m_show_debug_panel);
          ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
      }

      ImGui::Begin("Project");
      for (size_t i = 0; i < scene_files.size(); ++i) {
        if (ImGui::Selectable(scene_files[i].c_str(),
                              i == selected_scene_index)) {
          selected_scene_index = i;
        }
      }
      ImGui::End();

      if (m_show_landing_panel) {
        ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(),
                                ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

        ImGui::Begin("Welcome to Pulsar!", &m_show_landing_panel,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

        ImGui::Text(
            "Please open an existing project or create a new one to get "
            "started...");
        ImGui::Separator();

        if (ImGui::TreeNode("Useful Resources:")) {
          ImGui::BulletText("Pulsar Documentation");
          ImGui::Bullet();
          ImGui::SmallButton("Lua API Reference");
          ImGui::SameLine();
          ImGui::Text("Press me for more info.");
          ImGui::TreePop();
        }

        ImGui::End();
      }

      // ImGUI demo panel
      if (m_show_demo_panel) {
        ImGui::ShowDemoWindow(&m_show_demo_panel);
      }

      // Debug panel
      if (m_show_debug_panel) {
        ImGui::Begin("Debug panel", &m_show_debug_panel);
        ImGui::Text("User config path: %s", user_config_path.c_str());
        ImGui::Separator();
        ImGui::Text("Font path: %s", font_path.c_str());
        ImGui::Text("Font size: %f", font_size);
        ImGui::Text("Global font scaling %f", io.FontGlobalScale);
        ImGui::Text("UI scaling factor: %f", font_scaling_factor);
        ImGui::End();
      }
    }

    draw_editor_windows();
    draw_playback_controls();

    // Rendering
    ImGui::Render();

    SDL_SetRenderDrawColor(m_window->get_native_renderer(), 100, 100, 100, 255);
    SDL_RenderClear(m_window->get_native_renderer());
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(m_window->get_native_renderer());
  }

  return m_exit_status;
}

void App::Application::stop() {
  APP_PROFILE_FUNCTION();

  m_running = false;
}

void Application::on_event(const SDL_WindowEvent& event) {
  APP_PROFILE_FUNCTION();

  switch (event.event) {
    case SDL_WINDOWEVENT_CLOSE:
      return on_close();
    case SDL_WINDOWEVENT_MINIMIZED:
      return on_minimize();
    case SDL_WINDOWEVENT_SHOWN:
      return on_shown();
    default:
      // Do nothing otherwise
      return;
  }
}

void Application::on_minimize() {
  APP_PROFILE_FUNCTION();

  m_minimized = true;
}

void Application::on_shown() {
  APP_PROFILE_FUNCTION();

  m_minimized = false;
}

void Application::on_close() {
  APP_PROFILE_FUNCTION();

  stop();
}
void Application::open_project() {
  APP_DEBUG("trying to open project natively");
}

std::vector<std::string> Application::get_proj_scene_files() {
  std::vector<std::string> scene_files;
  auto game_path = Resources::game_path();
  game_path /= "scenes";

  for (const auto& entry : std::filesystem::directory_iterator(game_path)) {
    if (entry.path().extension() == ".scene") {
      scene_files.push_back(entry.path().filename().string());
    }
  }
  return scene_files;
}

void Application::draw_editor_windows() {
  ImGui::Begin("Editor");

  // Create dockspace
  ImGuiID dockspace_id = ImGui::GetID("EditorDockspace");
  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f),
                   ImGuiDockNodeFlags_PassthruCentralNode);

  // Scene Hierarchy
  ImGui::Begin("Hierarchy");
  for (const std::string& scene : scene_files) {
    if (ImGui::TreeNode(scene.c_str())) {
      for (const auto& actor :
           SceneManager::getInstance().copy_of_scene_actors) {
        if (ImGui::Selectable(actor->name.c_str(),
                              selected_actor == actor->name)) {
          selected_actor = actor->name;
          selected_scene = scene;
        }
      }
      ImGui::TreePop();
    }
  }
  ImGui::End();

  // Assets
  ImGui::Begin("Assets");

  // Load assets from the resources folder
  static std::vector<std::string> asset_paths;
  static std::vector<std::string> asset_names;
  // static std::vector<ImTextureID> asset_thumbnails;
  std::string resources_path = Resources::game_path().generic_string();
  if (asset_paths.empty()) {
    // Load assets from the resources folder
    std::vector<std::string> supported_extensions = {".png", ".jpg", ".jpeg",
                                                     ".bmp", ".wav", ".mp3"};

    for (const auto& entry :
         std::filesystem::recursive_directory_iterator(resources_path)) {
      if (entry.is_regular_file()) {
        std::string extension = entry.path().extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       ::tolower);

        if (std::find(supported_extensions.begin(), supported_extensions.end(),
                      extension) != supported_extensions.end()) {
          asset_paths.push_back(entry.path().string());
          asset_names.push_back(entry.path().filename().string());
        }
      }
    }
  }

  // Display assets
  static int selected_asset = -1;
  ImGui::BeginChild("AssetThumbnails", ImVec2(200, 0), true);
  for (size_t i = 0; i < asset_paths.size(); ++i) {
    // if (asset_thumbnails[i]) {
    //   ImGui::ImageButton(asset_thumbnails[i], ImVec2(64, 64));
    // } else {
    ImGui::Button(asset_names[i].c_str(), ImVec2(64, 64));
    // }

    if (ImGui::IsItemClicked()) {
      selected_asset = i;
    }

    ImGui::SameLine();
    ImGui::Text("%s", asset_names[i].c_str());
  }
  ImGui::EndChild();

  // Display asset details
  if (selected_asset >= 0) {
    ImGui::SameLine();
    ImGui::BeginChild("AssetDetails", ImVec2(0, 0), true);

    // if (asset_thumbnails[selected_asset]) {
    //   ImGui::ImageButton(asset_thumbnails[selected_asset], ImVec2(256, 256));
    // } else {
    ImGui::Text("[thumbnail placeholder] File: %s",
                asset_names[selected_asset].c_str());
    // }

    ImGui::EndChild();
  }

  ImGui::End();

  // Component Properties
  ImGui::Begin("Properties");
  if (!selected_actor.empty()) {
    for (const std::string& scene : scene_files) {
      if (scene == selected_scene) {
        for (const auto& actor :
             SceneManager::getInstance().copy_of_scene_actors) {
          if (actor->name == selected_actor) {
            ImGui::Text("Actor: %s", actor->name.c_str());
            ImGui::Separator();

            for (const auto& component : actor->entity_components_by_type) {
              if (ImGui::CollapsingHeader(component.first.c_str())) {
                if (component.first == "Transform") {
                  // Display Transform properties
                  static float position[3] = {0.0f, 0.0f, 0.0f};
                  ImGui::InputFloat3("Position", position);

                  static float rotation[3] = {0.0f, 0.0f, 0.0f};
                  ImGui::InputFloat3("Rotation", rotation);

                  static float scale[3] = {1.0f, 1.0f, 1.0f};
                  ImGui::InputFloat3("Scale", scale);
                } else if (component.first == "SpriteRenderer") {
                  // Display SpriteRenderer properties
                  static char image_path[256] = "";
                  ImGui::InputText("Image Path", image_path,
                                   sizeof(image_path));

                  static int sorting_order = 0;
                  ImGui::InputInt("Sorting Order", &sorting_order);

                  static float pivot_x = 0.5f;
                  ImGui::SliderFloat("Pivot X", &pivot_x, 0.0f, 1.0f);

                  static float pivot_y = 0.5f;
                  ImGui::SliderFloat("Pivot Y", &pivot_y, 0.0f, 1.0f);

                  ImGuiColorEditFlags misc_flags =
                      ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoDragDrop |
                      ImGuiColorEditFlags_AlphaPreview;
                  static ImVec4 color =
                      ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f,
                             200.0f / 255.0f);
                  ImGui::ColorEdit3("Color", (float*)&color, misc_flags);
                }
              }
            }
            ImGui::Separator();
            static bool addComponentModal = false;
            static char newComponentName[64] = "";

            if (ImGui::Button("Add Component")) {
              addComponentModal = true;
              memset(newComponentName, 0, sizeof(newComponentName));
            }

            if (addComponentModal) {
              ImGui::OpenPopup("Add Component");
              if (ImGui::BeginPopupModal("Add Component", &addComponentModal,
                                         ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::InputText("Component Name", newComponentName,
                                 sizeof(newComponentName));

                if (ImGui::Button("Create")) {
                  std::string componentPath =
                      resources_path + "/component_types/" +
                      std::string(newComponentName) + ".lua";
                  std::ofstream file(componentPath);
                  file << "-- New component" << std::endl;
                  file.close();
                  addComponentModal = false;
                }

                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                  addComponentModal = false;
                }

                ImGui::EndPopup();
              }
            }
            //            -----------
            static bool addComponentFromTemplateModal = false;
            static int selectedTemplate = -1;
            static char newComponentNameFromTemplate[64] = "";
            static std::vector<std::string> componentTemplates =
                LoadComponentTemplates();

            if (ImGui::Button("Add Component from Template")) {
              addComponentFromTemplateModal = true;
              selectedTemplate = -1;
              memset(newComponentNameFromTemplate, 0,
                     sizeof(newComponentNameFromTemplate));
              ImGui::OpenPopup("Add Component from Template");
            }
            if (addComponentFromTemplateModal) {
              if (ImGui::BeginPopupModal("Add Component from Template", nullptr,
                                         ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::ListBox(
                    "Templates", &selectedTemplate,
                    [](void* data, int idx, const char** out_text) {
                      auto* templates = (std::vector<std::string>*)data;
                      *out_text = (*templates)[idx].c_str();
                      strcpy(newComponentNameFromTemplate, *out_text);
                      return true;
                    },
                    (void*)&componentTemplates, componentTemplates.size(), 4);

                ImGui::InputText("Component Name", newComponentNameFromTemplate,
                                 sizeof(newComponentNameFromTemplate));

                if (ImGui::Button("Create") && selectedTemplate >= 0) {
                  std::string sourcePath =
                      resources_path + "/component_types/" +
                      componentTemplates[selectedTemplate] + ".lua";
                  std::string destPath =
                      resources_path + "/component_types/" +
                      std::string(newComponentNameFromTemplate) + ".lua";
                  std::filesystem::copy_file(
                      sourcePath, destPath,
                      std::filesystem::copy_options::overwrite_existing);
                  addComponentFromTemplateModal = false;
                  ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                  addComponentFromTemplateModal = false;
                  ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
              }
            }
          }
        }
      }
    }
  }
  ImGui::End();

  ImGui::End();  // End of Editor window`x`
}

void Application::draw_playback_controls() {
  ImGui::Begin("Playback Controls");

  if (ImGui::Button("Play")) {
    if (!game_thread.joinable()) {
      is_game_running = true;
      is_game_paused = false;
      start_game_thread();
    } else {
      is_game_paused = false;
      game_cv.notify_one();
    }
  }
  ImGui::SameLine();

  if (ImGui::Button("Pause")) {
    is_game_paused = true;
  }
  ImGui::SameLine();

  if (ImGui::Button("Stop")) {
    is_game_running = false;
    is_game_paused = false;
    game_cv.notify_one();
    stop_game_thread();
  }

  ImGui::End();
}

void Application::run_game_thread() {
  const std::string game_config_path =
      (Resources::game_path() / "game.config").generic_string();
  rapidjson::Document game_config;
  EngineUtils::ReadJsonFile(game_config_path, game_config);

  SceneManager& scene_manager = SceneManager::getInstance();
  Renderer& renderer = Renderer::getInstance();

  renderer.initialize(game_config);
  CameraManager::initialize();
  AudioManager::initialize();
  AudioManager::start_intro_music(game_config);
  InputManager::InitKeyToScancodeMap();
  InputManager::Init();

  while (is_game_running) {
    std::unique_lock<std::mutex> lock(game_mutex);
    game_cv.wait(lock, [&] { return !is_game_paused; });
    lock.unlock();

    SDL_Event input_event;
    while (SDL_PollEvent(&input_event)) {
      InputManager::ProcessEvent(input_event);
      if (input_event.type == SDL_QUIT) {
        is_game_running = false;
        break;
      }
    }
    SDL_RenderClear(renderer.get_sdl_renderer());
    AudioManager::stop_intro_music();

    scene_manager.update_scene_actors();

    if (SceneManager::latest_scene_change_request) {
      scene_manager.trigger_scene_change(
          *SceneManager::latest_scene_change_request);
      SceneManager::latest_scene_change_request.reset();
    }

    EventBus::ProcessPendingSubscriptions();
    EventBus::ProcessPendingUnsubscriptions();

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

void Application::start_game_thread() {
  if (!game_thread.joinable()) {
    game_thread = std::thread(&Application::run_game_thread, this);
  }
}

void Application::stop_game_thread() {
  if (game_thread.joinable()) {
    game_thread.join();
  }
}
std::vector<std::string> Application::LoadComponentTemplates() {
  std::vector<std::string> templates;
  auto comp_path = Resources::game_path();
  comp_path /= "component_types";

  for (const auto& entry : std::filesystem::directory_iterator(comp_path)) {
    if (entry.is_regular_file() and entry.path().extension() == ".lua") {
      templates.push_back(entry.path().stem().string());
    }
  }

  return templates;
}

}  // namespace App
