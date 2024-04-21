//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_ECS_H_
#define PULSAR_SRC_ENGINE_CORE_ECS_H_

#include <unordered_map>
#include <utility>
#include "Core/Resources.hpp"

// clang-format off
#include "lua.hpp"
#include <LuaBridge/LuaBridge.h>
// clang-format on

class Actor;

typedef std::unordered_map<std::string, luabridge::LuaRef> base_component_map;
typedef std::vector<std::pair<Actor*, luabridge::LuaRef>> actor_component_list;
typedef std::unordered_map<size_t, Actor*> actor_id_map;
typedef std::vector<std::pair<Actor*, std::string>> actor_component_key_list;
namespace App {

class ECS {
  lua_State* lua_state = nullptr;
  base_component_map component_registry;
  // for the singleton pattern
  ECS() {}
  ~ECS() {}

 public:
  enum class ComponentType { CPP, LUA };
  ECS(const ECS&) = delete;
  ECS& operator=(const ECS&) = delete;
  ECS(ECS&&) = delete;
  ECS& operator=(ECS&&) = delete;

  static ECS& getInstance() {
    static ECS instance;
    return instance;
  }

  [[nodiscard]] lua_State* get_lua_state() const { return lua_state; }

  [[nodiscard]] base_component_map get_component_registry() const {
    return component_registry;
  }

  const std::filesystem::path COMPONENTS_DIR = Resources::game_path() / "component_types";

  void initialize();
  void reset();
  void initialize_state();
  void initialize_component_registry();
  void initialize_functions();
  void establish_inheritance(const luabridge::LuaRef& child_table,
                             const luabridge::LuaRef& parent_table) const;
  std::pair<ComponentType, luabridge::LuaRef> create_component(
      const std::string&,
      const std::string&);

  void reg_debug_namespace();
  void reg_application_namespace();
  void reg_audio_manager();
  void reg_camera_manager();
  void reg_renderer();
  void reg_vector2();
  void reg_input_manager();
  void reg_scene_manager();
  void reg_actor_class();
  void reg_actor_static_namespace();
  void reg_rigidbody_class();
  void reg_contact_class();
  void reg_raycaster_class();
  void reg_eventbus_class();

  // [[nodiscard]] std::string get_component_path(const std::string
  // &component_name) const;
  static void Lua_Log(const std::string& message);
  static void Lua_LogError(const std::string& message);
  static int Lua_App_GetFrame();
  static void Lua_App_Sleep(int ms);
  static void Lua_App_Quit();
  static void Lua_App_OpenURL(const std::string& url);
};


}  // namespace App

#endif  // PULSAR_SRC_ENGINE_CORE_ECS_H_
