#define SDL_MAIN_HANDLED

#include <exception>
#include <memory>

#include "Core/Application.hpp"
#include "Core/UI.h"
#include "Core/Debug/Instrumentor.hpp"
#include "Core/Log.hpp"

int main() {
  try {
    APP_PROFILE_BEGIN_SESSION_WITH_FILE("App", "profile.json");

    {
      APP_PROFILE_SCOPE("Pulsar scope");
      App::Application app{"Pulsar"};
      App::UI app_ui;
      app.set_UI(std::make_unique<App::UI>(app_ui));
      app.run();
    }

    APP_PROFILE_END_SESSION();
  } catch (std::exception& e) {
    APP_ERROR("Main process terminated with: {}", e.what());
  }

  return 0;
}
