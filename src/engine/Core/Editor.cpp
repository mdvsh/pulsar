////
//// Created by Madhav Shekhar Sharma on 4/20/24.
////
//
//#include "Editor.h"
//#include <SDL2/SDL.h>
//#include <imgui.h>
//
//#include "Settings/Project.hpp"
//
//namespace App {
//
//void Editor::Init(const Zep::NVec2f& pixelScale) {
//  const std::string config_path =
//      SDL_GetPrefPath(App::COMPANY_NAMESPACE.c_str(), App::APP_NAME.c_str());
//  m_spZep = std::make_shared<Zep::ZepWrapper>(
//      config_path, Zep::NVec2f(pixelScale.x, pixelScale.y),
//      []([[maybe_unused]] const std::shared_ptr<Zep::ZepMessage>& spMessage) -> void {});
//
//  auto& display = m_spZep->GetEditor().GetDisplay();
//  auto pImFont = ImGui::GetIO().Fonts[0].Fonts[0];
//  auto pixelHeight = pImFont->FontSize;
//  display.SetFont(Zep::ZepTextType::UI, std::make_shared<Zep::ZepFont_ImGui>(
//                                       display, pImFont, int(pixelHeight)));
//  display.SetFont(Zep::ZepTextType::Text, std::make_shared<Zep::ZepFont_ImGui>(
//                                         display, pImFont, int(pixelHeight)));
//  display.SetFont(Zep::ZepTextType::Heading1,
//                  std::make_shared<Zep::ZepFont_ImGui>(display, pImFont,
//                                                  int(pixelHeight * 1.5)));
//  display.SetFont(Zep::ZepTextType::Heading2,
//                  std::make_shared<Zep::ZepFont_ImGui>(display, pImFont,
//                                                  int(pixelHeight * 1.25)));
//  display.SetFont(Zep::ZepTextType::Heading3,
//                  std::make_shared<Zep::ZepFont_ImGui>(display, pImFont,
//                                                  int(pixelHeight * 1.125)));
//}
//
//void Editor::Update() {
//  if (m_spZep) {
//    m_spZep->GetEditor().RefreshRequired();
//  }
//}
//
//void Editor::Load(const std::filesystem::path& file) {
//  m_spZep->GetEditor().InitWithFileOrDir(file.string());
//}
//
//void Editor::Shutdown() {
//  m_spZep.reset();
//}
//
//void Editor::Render(const Zep::NVec2i& displaySize) {
//  ImGui::SetNextWindowSize(ImVec2(displaySize.x, displaySize.y),
//                           ImGuiCond_FirstUseEver);
//  if (!ImGui::Begin("Editor", nullptr,
//                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar)) {
//    ImGui::End();
//    return;
//  }
//
//  auto min = ImGui::GetCursorScreenPos();
//  auto max = ImGui::GetContentRegionAvail();
//  if (max.x <= 0)
//    max.x = 1;
//  if (max.y <= 0)
//    max.y = 1;
//  ImGui::InvisibleButton("ZepContainer", max);
//
//  max.x = min.x + max.x;
//  max.y = min.y + max.y;
//
//  m_spZep->GetEditor().SetDisplayRegion(Zep::NVec2f(min.x, min.y),
//                                        Zep::NVec2f(max.x, max.y));
//  m_spZep->GetEditor().Display();
//
//  bool focused = ImGui::IsWindowFocused();
//  if (focused) {
//    m_spZep->HandleInput();
//  }
//
//  static int focus_count = 0;
//  if (focus_count++ < 2) {
//    ImGui::SetWindowFocus();
//  }
//
//  ImGui::End();
//}
//
//}  // namespace App