//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_INPUTMANAGER_H_
#define PULSAR_SRC_ENGINE_CORE_INPUTMANAGER_H_

#include <SDL2/SDL.h>
#include <glm/vec2.hpp>
#include <unordered_map>
#include <vector>

#include "Engine.h"

enum INPUT_STATE {
  INPUT_STATE_UP,
  INPUT_STATE_JUST_BECAME_DOWN,
  INPUT_STATE_DOWN,
  INPUT_STATE_JUST_BECAME_UP
};

class InputManager {
 public:
  // InputManager(Engine *engine, Renderer *renderer) : engine(engine),
  // renderer(renderer) {}
  static void ProcessEvent(
      const SDL_Event&
          input_event);  // Call every frame at start of event loop.
  static void Init();
  static void LateUpdate();

  static bool GetKey(const std::string& key);
  static bool GetKeyDown(const std::string& key);
  static bool GetKeyUp(const std::string& key);

  static std::unordered_map<SDL_Keycode, glm::vec2> key_directions_map;

  static glm::vec2 GetMousePosition();
  static bool GetMouseButton(int button);
  static bool GetMouseButtonDown(int button);
  static bool GetMouseButtonUp(int button);
  static float GetMouseScrollDelta();

  static std::unordered_map<std::string, SDL_Scancode> key_to_scancode_map;
  [[nodiscard]] static SDL_Scancode key_to_scancode(const std::string& key);

 private:
  static inline std::unordered_map<SDL_Scancode, INPUT_STATE> keyboard_states;
  static inline std::vector<SDL_Scancode> just_became_down_scancodes;
  static inline std::vector<SDL_Scancode> just_became_up_scancodes;

  static inline glm::vec2 mouse_position;
  static inline std::unordered_map<int, INPUT_STATE> mouse_button_states;
  static inline std::vector<int> just_became_down_buttons;
  static inline std::vector<int> just_became_up_buttons;
  static inline float mouse_scroll_this_frame = 0.0f;

  // Engine *engine = nullptr;
  // Renderer *renderer = nullptr;
};


#endif  // PULSAR_SRC_ENGINE_CORE_INPUTMANAGER_H_
