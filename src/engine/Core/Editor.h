////
//// Created by Madhav Shekhar Sharma on 4/20/24.
////
//
//#ifndef PULSAR_SRC_ENGINE_CORE_EDITOR_H_
//#define PULSAR_SRC_ENGINE_CORE_EDITOR_H_
//
//#include <zep.h>
//#include <filesystem>
//#include <string>
//#include <memory>
//
//namespace Zep {
//using cmdFunc = std::function<void(const std::vector<std::string>&)>;
//class [[maybe_unused]] ZepCmd : public ZepExCommand {
// public:
//  ZepCmd(ZepEditor& editor, const std::string name, cmdFunc fn)
//      : ZepExCommand(editor), m_name(name), m_func(fn) {}
//
//  virtual void Run(const std::vector<std::string>& args) override {
//    m_func(args);
//  }
//
//  virtual const char* ExCommandName() const override { return m_name.c_str(); }
//
// private:
//  std::string m_name;
//  cmdFunc m_func;
//};
//
//struct ZepWrapper : public Zep::IZepComponent {
//  ZepWrapper(const std::filesystem::path& root_path,
//             const Zep::NVec2f& pixelScale,
//             std::function<void(std::shared_ptr<Zep::ZepMessage>)> fnCommandCB)
//      : zepEditor(std::filesystem::path(root_path.string()), pixelScale),
//        Callback(fnCommandCB) {
//    zepEditor.RegisterCallback(this);
//  }
//
//  virtual ~ZepWrapper() = default;
//
//  virtual Zep::ZepEditor& GetEditor() const override {
//    return (Zep::ZepEditor&)zepEditor;
//  }
//
//  virtual void Notify(std::shared_ptr<Zep::ZepMessage> message) override {
//    Callback(message);
//
//    return;
//  }
//
//  virtual void HandleInput() { zepEditor.HandleInput(); }
//
//  Zep::ZepEditor_ImGui zepEditor;
//  std::function<void(std::shared_ptr<Zep::ZepMessage>)> Callback;
//};
//} // namespace Zep
//
//namespace App {
//
//class Editor {
//  public:
//    Editor() = default;
//    ~Editor() = default;
//
//    void Init(const Zep::NVec2f& pixelScale);
//    void Update();
//    void Render(const Zep::NVec2i& windowSize);
//    void Load(const std::filesystem::path& file);
//    void Shutdown();
//
//  private:
//   std::shared_ptr<Zep::ZepWrapper> m_spZep{nullptr};
//};
//
//}  // namespace App
//
//#endif  // PULSAR_SRC_ENGINE_CORE_EDITOR_H_
