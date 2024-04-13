//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#include "InputManager.h"

std::unordered_map<std::string, SDL_Scancode> InputManager::key_to_scancode_map;

void InputManager::ProcessEvent(const SDL_Event& input_event) {
  const auto key_scancode = input_event.key.keysym.scancode;
  if (input_event.type == SDL_KEYDOWN) {
    keyboard_states[key_scancode] = INPUT_STATE_JUST_BECAME_DOWN;
    just_became_down_scancodes.push_back(key_scancode);
  } else if (input_event.type == SDL_KEYUP) {
    keyboard_states[key_scancode] = INPUT_STATE_JUST_BECAME_UP;
    just_became_up_scancodes.push_back(key_scancode);
  } else if (input_event.type == SDL_MOUSEMOTION) {
    mouse_position.x = static_cast<float>(input_event.motion.x);
    mouse_position.y = static_cast<float>(input_event.motion.y);
  } else if (input_event.type == SDL_MOUSEBUTTONDOWN) {
    mouse_button_states[input_event.button.button] =
        INPUT_STATE_JUST_BECAME_DOWN;
    just_became_down_buttons.push_back(input_event.button.button);
  } else if (input_event.type == SDL_MOUSEBUTTONUP) {
    mouse_button_states[input_event.button.button] = INPUT_STATE_JUST_BECAME_UP;
    just_became_up_buttons.push_back(input_event.button.button);
  } else if (input_event.type == SDL_MOUSEWHEEL) {
    mouse_scroll_this_frame += input_event.wheel.preciseY;
  }
}

void InputManager::Init() {
  for (int code = SDL_SCANCODE_UNKNOWN; code < SDL_NUM_SCANCODES; code++) {
    keyboard_states[static_cast<SDL_Scancode>(code)] = INPUT_STATE_UP;
  }
  mouse_position = glm::vec2(0.0f, 0.0f);
}

void InputManager::LateUpdate() {
  // handling keyboard
  for (const auto& scancode : just_became_down_scancodes)
    keyboard_states[scancode] = INPUT_STATE_DOWN;
  just_became_down_scancodes.clear();

  for (const auto& scancode : just_became_up_scancodes)
    keyboard_states[scancode] = INPUT_STATE_UP;
  just_became_up_scancodes.clear();

  // handling mouse
  for (const int button : just_became_down_buttons)
    mouse_button_states[button] = INPUT_STATE_DOWN;
  just_became_down_buttons.clear();

  for (const int button : just_became_up_buttons)
    mouse_button_states[button] = INPUT_STATE_UP;
  just_became_up_buttons.clear();

  mouse_scroll_this_frame = 0.0f;
}
SDL_Scancode InputManager::key_to_scancode(const std::string& key) {
  if (key_to_scancode_map.find(key) != key_to_scancode_map.end()) {
    return key_to_scancode_map[key];
  } else {
    return SDL_SCANCODE_UNKNOWN;
  }
}

bool InputManager::GetKey(const std::string& key) {
  const auto scancode = key_to_scancode(key);
  if (scancode == SDL_SCANCODE_UNKNOWN)
    return false;
  return keyboard_states[scancode] == INPUT_STATE_DOWN ||
         keyboard_states[scancode] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool InputManager::GetKeyDown(const std::string& key) {
  const auto scancode = key_to_scancode(key);
  if (scancode == SDL_SCANCODE_UNKNOWN)
    return false;
  return keyboard_states[scancode] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool InputManager::GetKeyUp(const std::string& key) {
  const auto scancode = key_to_scancode(key);
  if (scancode == SDL_SCANCODE_UNKNOWN)
    return false;
  return keyboard_states[scancode] == INPUT_STATE_JUST_BECAME_UP;
}
glm::vec2 InputManager::GetMousePosition() {
  return mouse_position;
}
bool InputManager::GetMouseButton(const int button) {
  return mouse_button_states[button] == INPUT_STATE_DOWN ||
         mouse_button_states[button] == INPUT_STATE_JUST_BECAME_DOWN;
}
bool InputManager::GetMouseButtonDown(const int button) {
  return mouse_button_states[button] == INPUT_STATE_JUST_BECAME_DOWN;
}
bool InputManager::GetMouseButtonUp(const int button) {
  return mouse_button_states[button] == INPUT_STATE_JUST_BECAME_UP;
}
float InputManager::GetMouseScrollDelta() {
  return mouse_scroll_this_frame;
}

void InputManager::InitKeyToScancodeMap() {
  key_to_scancode_map = {
    // Directional (arrow) Keys
    {"up", SDL_SCANCODE_UP},
    {"down", SDL_SCANCODE_DOWN},
    {"right", SDL_SCANCODE_RIGHT},
    {"left", SDL_SCANCODE_LEFT},

    // Misc Keys
    {"escape", SDL_SCANCODE_ESCAPE},

    // Modifier Keys
    {"lshift", SDL_SCANCODE_LSHIFT},
    {"rshift", SDL_SCANCODE_RSHIFT},
    {"lctrl", SDL_SCANCODE_LCTRL},
    {"rctrl", SDL_SCANCODE_RCTRL},
    {"lalt", SDL_SCANCODE_LALT},
    {"ralt", SDL_SCANCODE_RALT},

    // Editing Keys
    {"tab", SDL_SCANCODE_TAB},
    {"return", SDL_SCANCODE_RETURN},
    {"enter", SDL_SCANCODE_RETURN},
    {"backspace", SDL_SCANCODE_BACKSPACE},
    {"delete", SDL_SCANCODE_DELETE},
    {"insert", SDL_SCANCODE_INSERT},

    // Character Keys
    {"space", SDL_SCANCODE_SPACE},
    {"a", SDL_SCANCODE_A},
    {"b", SDL_SCANCODE_B},
    {"c", SDL_SCANCODE_C},
    {"d", SDL_SCANCODE_D},
    {"e", SDL_SCANCODE_E},
    {"f", SDL_SCANCODE_F},
    {"g", SDL_SCANCODE_G},
    {"h", SDL_SCANCODE_H},
    {"i", SDL_SCANCODE_I},
    {"j", SDL_SCANCODE_J},
    {"k", SDL_SCANCODE_K},
    {"l", SDL_SCANCODE_L},
    {"m", SDL_SCANCODE_M},
    {"n", SDL_SCANCODE_N},
    {"o", SDL_SCANCODE_O},
    {"p", SDL_SCANCODE_P},
    {"q", SDL_SCANCODE_Q},
    {"r", SDL_SCANCODE_R},
    {"s", SDL_SCANCODE_S},
    {"t", SDL_SCANCODE_T},
    {"u", SDL_SCANCODE_U},
    {"v", SDL_SCANCODE_V},
    {"w", SDL_SCANCODE_W},
    {"x", SDL_SCANCODE_X},
    {"y", SDL_SCANCODE_Y},
    {"z", SDL_SCANCODE_Z},
    {"0", SDL_SCANCODE_0},
    {"1", SDL_SCANCODE_1},
    {"2", SDL_SCANCODE_2},
    {"3", SDL_SCANCODE_3},
    {"4", SDL_SCANCODE_4},
    {"5", SDL_SCANCODE_5},
    {"6", SDL_SCANCODE_6},
    {"7", SDL_SCANCODE_7},
    {"8", SDL_SCANCODE_8},
    {"9", SDL_SCANCODE_9},
    {"/", SDL_SCANCODE_SLASH},
    {";", SDL_SCANCODE_SEMICOLON},
    {"=", SDL_SCANCODE_EQUALS},
    {"-", SDL_SCANCODE_MINUS},
    {".", SDL_SCANCODE_PERIOD},
    {",", SDL_SCANCODE_COMMA},
    {"[", SDL_SCANCODE_LEFTBRACKET},
    {"]", SDL_SCANCODE_RIGHTBRACKET},
    {"\\", SDL_SCANCODE_BACKSLASH},
    {"'", SDL_SCANCODE_APOSTROPHE}
  };
}
