//
// Created by Madhav Shekhar Sharma on 4/16/24.
//

#include "UI.h"
#include "Core/SceneManager.h"

namespace App {
void UI::renderUI() {
  set_proj_scene_files();
  drawToolbar();
  ImGui::Begin("Editor");
  drawEditorPane();
  drawAssetsPane();
  drawCompPropsPane();
  drawNewCompPane();
  ImGui::End();
}

void UI::drawToolbar() {
  ImGui::DockSpaceOverViewport();
  dockspace_id = ImGui::GetID("MainDockSpace");
  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f),
                   ImGuiDockNodeFlags_PassthruCentralNode);

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Open Project", "Cmd+O")) {
        // open_project();
      }
      if (ImGui::MenuItem("Exit", "Cmd+Q")) {
        // stop();
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

  // -----next-----

  ImGui::Begin("Project");
  for (const std::string& scene : scene_files) {
    if (ImGui::Selectable(scene.c_str(), scene == selected_scene)) {
      selected_scene = scene;
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
  if (m_show_demo_panel) {
    ImGui::ShowDemoWindow(&m_show_demo_panel);
  }
}

void UI::drawEditorPane() {
  // ImGui::Begin("Editor");
  dockspace_id = ImGui::GetID("EditorDockspace");
  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f),
                   ImGuiDockNodeFlags_PassthruCentralNode);

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
}

void UI::drawAssetsPane() {
  ImGui::Begin("Assets");
  static std::vector<std::string> asset_paths;
  static std::vector<std::string> asset_names;
  // static std::vector<ImTextureID> asset_thumbnails;
  if (asset_paths.empty()) {
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
}

void UI::drawCompPropsPane() {
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
            drawNewCompPane();
          }
        }
      }
    }
  }
  ImGui::End();
}

void UI::drawNewCompPane() {
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
        std::string componentPath = resources_path_str + "/component_types/" +
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

  static bool addComponentFromTemplateModal = false;
  static int selectedTemplate = -1;
  static char newComponentNameFromTemplate[64] = "";
  static std::vector<std::string> componentTemplates;

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
        std::string sourcePath = resources_path_str + "/component_types/" +
                                 componentTemplates[selectedTemplate] + ".lua";
        std::string destPath = resources_path_str + "/component_types/" +
                               std::string(newComponentNameFromTemplate) +
                               ".lua";
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
void UI::drawCenterPane() {}

}  // namespace App