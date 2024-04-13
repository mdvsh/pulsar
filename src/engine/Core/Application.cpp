#include "Application.hpp"

#include <SDL2/SDL.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>
#include <imgui.h>

#include <memory>
#include <string>

#include "Core/DPIHandler.hpp"
#include "Core/Debug/Instrumentor.hpp"
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
  // m_engine = std::make_unique<Engine>();
  // m_engine->load_initial_settings();
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

}  // namespace App
