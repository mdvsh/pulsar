//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#include <filesystem>
#include <functional>
#include <thread>

#include "ECS.h"
#include "EventBus.h"
#include "AudioManager.h"
#include "CameraManager.h"
#include "Renderer.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Actor.h"
#include "Rigidbody.h"
#include "Raycaster.h"

namespace App {

void ECS::initialize() {
  initialize_state();
  initialize_component_registry();
  initialize_functions();
}

void ECS::initialize_state() {
  lua_state = luaL_newstate();
  luaL_openlibs(lua_state);
}

void ECS::initialize_functions() {
  reg_debug_namespace();
  reg_application_namespace();
  reg_audio_manager();
  reg_camera_manager();
  reg_renderer();
  reg_vector2();
  reg_input_manager();
  reg_scene_manager();
  reg_actor_class();
  reg_rigidbody_class();
  reg_actor_static_namespace();
  reg_contact_class();
  reg_raycaster_class();
  reg_eventbus_class();
}

void ECS::initialize_component_registry() {
  if (lua_state == nullptr) {
    std::cout << "error: ECS not initialized";
    std::exit(0);
  }
  for (const auto& entry :
       std::filesystem::directory_iterator(COMPONENTS_DIR)) {
    if (entry.is_directory())
      continue;
    if (entry.path().extension() != ".lua")
      continue;

    const std::string component_name = entry.path().stem().string();
    if (luaL_dofile(lua_state, entry.path().string().c_str()) != LUA_OK) {
      std::cout << "problem with lua file " << component_name;
      std::exit(0);
    }

    luabridge::LuaRef component_table =
        luabridge::getGlobal(lua_state, component_name.c_str());
    component_registry.insert({component_name, component_table});
  }
}

void ECS::establish_inheritance(const luabridge::LuaRef& child_table,
                                const luabridge::LuaRef& parent_table) const {
  luabridge::LuaRef new_metatable = luabridge::newTable(lua_state);
  new_metatable["__index"] = parent_table;

  child_table.push(lua_state);
  new_metatable.push(lua_state);
  lua_setmetatable(lua_state, -2);
  lua_pop(lua_state, 1);
}
void ECS::reg_debug_namespace() {
  luabridge::getGlobalNamespace(lua_state)
      .beginNamespace("Debug")
      .addFunction("Log", &ECS::Lua_Log)
      .addFunction("LogError", &ECS::Lua_LogError)
      .endNamespace();
}
void ECS::reg_application_namespace() {
  luabridge::getGlobalNamespace(lua_state)
      .beginNamespace("Application")
      .addFunction("Quit", &Lua_App_Quit)
      .addFunction("Sleep", &Lua_App_Sleep)
      .addFunction("GetFrame", &Lua_App_GetFrame)
      .addFunction("OpenURL", &Lua_App_OpenURL)
      .endNamespace();
}
void ECS::reg_audio_manager() {
  luabridge::getGlobalNamespace(lua_state)
      .beginClass<AudioManager>("Audio")
      .addStaticFunction("Play", &AudioManager::LuaPlayChannel)
      .addStaticFunction("Halt", &AudioManager::halt_channel)
      .addStaticFunction("SetVolume", &AudioManager::SetVolume)
      .endClass();
}
void ECS::reg_camera_manager() {
  luabridge::getGlobalNamespace(lua_state)
      .beginClass<CameraManager>("Camera")
      .addStaticFunction("SetPosition", &CameraManager::set_position)
      .addStaticFunction("GetPositionX", &CameraManager::get_position_x)
      .addStaticFunction("GetPositionY", &CameraManager::get_position_y)
      .addStaticFunction("SetZoom", &CameraManager::set_zoom)
      .addStaticFunction("GetZoom", &CameraManager::get_zoom)
      .endClass();
}
void ECS::reg_renderer() {
  luabridge::getGlobalNamespace(lua_state)
      .beginClass<Renderer>("Image")
      .addStaticFunction("DrawUI", &Renderer::LuaDrawUI)
      .addStaticFunction("DrawUIEx", &Renderer::LuaDrawUIEx)
      .addStaticFunction("Draw", &Renderer::LuaDraw)
      .addStaticFunction("DrawEx", &Renderer::LuaDrawEx)
      .addStaticFunction("DrawPixel", &Renderer::LuaDrawPixel)
      .endClass()

      .beginClass<Renderer>("Text")
      .addStaticFunction("Draw", &Renderer::LuaRenderText)
      .endClass();
}
void ECS::reg_input_manager() {
  luabridge::getGlobalNamespace(lua_state)
      .beginClass<InputManager>("Input")
      .addStaticFunction("GetKey", &InputManager::GetKey)
      .addStaticFunction("GetKeyDown", &InputManager::GetKeyDown)
      .addStaticFunction("GetKeyUp", &InputManager::GetKeyUp)
      .addStaticFunction("GetMousePosition", &InputManager::GetMousePosition)
      .addStaticFunction("GetMouseButton", &InputManager::GetMouseButton)
      .addStaticFunction("GetMouseButtonDown",
                         &InputManager::GetMouseButtonDown)
      .addStaticFunction("GetMouseButtonUp", &InputManager::GetMouseButtonUp)
      .addStaticFunction("GetMouseScrollDelta",
                         &InputManager::GetMouseScrollDelta)
      .endClass();
}
void ECS::reg_scene_manager() {
  luabridge::getGlobalNamespace(lua_state)
      .beginClass<SceneManager>("Scene")
      .addStaticFunction("Load", &SceneManager::LuaLoadNewScene)
      .addStaticFunction("GetCurrent", &SceneManager::LuaGetCurrentScene)
      .addStaticFunction("DontDestroy", &SceneManager::LuaPersistActor)
      .endClass()

      .beginClass<glm::vec2>("vec2")
      .addProperty("x", &glm::vec2::x)
      .addProperty("y", &glm::vec2::y)
      .endClass();
}
void ECS::reg_actor_class() {
  luabridge::getGlobalNamespace(lua_state)
      .beginClass<Actor>("Actor")
      .addFunction("GetName", &Actor::GetName)
      .addFunction("GetID", &Actor::GetID)
      .addFunction("GetComponentByKey", &Actor::GetComponentByKey)
      // Note: these queries are by type
      .addFunction("GetComponent", &Actor::GetComponent)
      .addFunction("GetComponents", &Actor::GetComponents)
      .addFunction("AddComponent", &Actor::LuaAddComponent)
      .addFunction("RemoveComponent", &Actor::LuaRemoveComponent)
      .endClass();
}
void ECS::reg_actor_static_namespace() {
  luabridge::getGlobalNamespace(lua_state)
      .beginNamespace("Actor")
      .addFunction("Find", &SceneManager::GetActor)
      .addFunction("FindAll", &SceneManager::GetActors)
      .addFunction("Instantiate", &Actor::LuaCreateActor)
      .addFunction("Destroy", &Actor::LuaDestroyActor)
      .endNamespace();
}
void ECS::reg_rigidbody_class() {
  luabridge::getGlobalNamespace(lua_state)
      .beginClass<Rigidbody>("Rigidbody")
      .addData("x", &Rigidbody::x)
      .addData("y", &Rigidbody::y)
      .addData("body_type", &Rigidbody::body_type)
      .addData("precise", &Rigidbody::precise)
      .addData("gravity_scale", &Rigidbody::gravity_scale)
      .addData("density", &Rigidbody::density)
      .addData("angular_friction", &Rigidbody::angular_friction)
      .addData("rotation", &Rigidbody::rotation)
      .addData("has_collider", &Rigidbody::has_collider)
      .addData("has_trigger", &Rigidbody::has_trigger)
      .addData("type", &Rigidbody::type)
      .addData("key", &Rigidbody::key)
      .addData("actor", &Rigidbody::actor)
      .addData("enabled", &Rigidbody::enabled)
      .addData("collider_type", &Rigidbody::collider_type)
      .addData("width", &Rigidbody::collider_width)
      .addData("height", &Rigidbody::collider_height)
      .addData("radius", &Rigidbody::collider_radius)
      .addData("bounciness", &Rigidbody::bounciness)
      .addData("friction", &Rigidbody::friction)

      .addData("trigger_type", &Rigidbody::trigger_type)
      .addData("trigger_width", &Rigidbody::trigger_width)
      .addData("trigger_height", &Rigidbody::trigger_height)
      .addData("trigger_radius", &Rigidbody::trigger_radius)

      .addFunction("OnStart", &Rigidbody::Initialize)
      .addFunction("OnDestroy", &Rigidbody::Destroy)
      .addFunction("GetPosition", &Rigidbody::GetPosition)
      .addFunction("SetPosition", &Rigidbody::SetPosition)
      .addFunction("GetRotation", &Rigidbody::GetRotation)
      .addFunction("SetRotation", &Rigidbody::SetRotation)

      .addFunction("AddForce", &Rigidbody::AddForce)
      .addFunction("SetVelocity", &Rigidbody::SetVelocity)
      .addFunction("SetAngularVelocity", &Rigidbody::SetAngularVelocity)
      .addFunction("SetGravityScale", &Rigidbody::SetGravityScale)
      .addFunction("SetUpDirection", &Rigidbody::SetUpDirection)
      .addFunction("SetRightDirection", &Rigidbody::SetRightDirection)
      .addFunction("GetVelocity", &Rigidbody::GetVelocity)
      .addFunction("GetAngularVelocity", &Rigidbody::GetAngularVelocity)
      .addFunction("GetGravityScale", &Rigidbody::GetGravityScale)
      .addFunction("GetUpDirection", &Rigidbody::GetUpDirection)
      .addFunction("GetRightDirection", &Rigidbody::GetRightDirection)
      .endClass();
}
void ECS::reg_vector2() {
  luabridge::getGlobalNamespace(lua_state)
      .beginClass<b2Vec2>("Vector2")
      .addConstructor<void (*)(float, float)>()
      .addProperty("x", &b2Vec2::x)
      .addProperty("y", &b2Vec2::y)
      .addFunction("Normalize", &b2Vec2::Normalize)
      .addFunction("Length", &b2Vec2::Length)
      .addFunction("__add",
                   std::function([](const b2Vec2* v1, const b2Vec2& v2) {
                     return b2Vec2(v1->x + v2.x, v1->y + v2.y);
                   }))
      .addFunction("__sub",
                   std::function([](const b2Vec2* v1, const b2Vec2& v2) {
                     return b2Vec2(v1->x - v2.x, v1->y - v2.y);
                   }))
      .addFunction("__mul", std::function([](const b2Vec2* v, float s) {
                     return b2Vec2(v->x * s, v->y * s);
                   }))
      .addStaticFunction("Distance", &b2Distance)
      .addStaticFunction(
          "Dot", static_cast<float (*)(const b2Vec2&, const b2Vec2&)>(&b2Dot))
      .endClass();
}
void ECS::reg_contact_class() {
  luabridge::getGlobalNamespace(lua_state)
      .beginClass<Contact>("Collision")
      .addConstructor<void (*)(void)>()
      .addProperty("other", &Contact::GetOther, &Contact::SetOther)
      .addProperty("point", &Contact::GetPoint, &Contact::SetPoint)
      .addProperty("relative_velocity", &Contact::GetRelativeVelocity,
                   &Contact::SetRelativeVelocity)
      .addProperty("normal", &Contact::GetNormal, &Contact::SetNormal)
      .endClass();
}

void ECS::reg_raycaster_class() {
  luabridge::getGlobalNamespace(lua_state)
      .beginClass<RaycastResult>("HitResult")
      .addConstructor<void (*)(void)>()
      .addProperty("actor", &RaycastResult::GetActor, &RaycastResult::SetActor)
      .addProperty("point", &RaycastResult::GetPoint, &RaycastResult::SetPoint)
      .addProperty("normal", &RaycastResult::GetNormal,
                   &RaycastResult::SetNormal)
      .addProperty("is_trigger", &RaycastResult::IsTrigger,
                   &RaycastResult::SetTrigger)
      .endClass()

      .beginClass<Raycaster>("Physics")
      .addStaticFunction("Raycast", &Raycaster::LuaRaycast)
      .addStaticFunction("RaycastAll", &Raycaster::LuaRaycastAll)
      .endClass();
}
void ECS::reg_eventbus_class() {
  luabridge::getGlobalNamespace(lua_state)
      .beginClass<EventBus>("Event")
      .addStaticFunction("Publish", &EventBus::Publish)
      .addStaticFunction("Subscribe", &EventBus::Subscribe)
      .addStaticFunction("Unsubscribe", &EventBus::Unsubscribe)
      .endClass();
}

std::pair<ECS::ComponentType, luabridge::LuaRef> ECS::create_component(
    const std::string& key,
    const std::string& name) {
  if (name == "Rigidbody") {
    // Special C++ component
    SceneManager::getInstance().CreatePhysWorld();
    auto* rigidbody = new Rigidbody();
    rigidbody->key = key;
    rigidbody->type = name;
    luabridge::LuaRef component(lua_state, rigidbody);
    component["key"] = key;
    component["type"] = name;
    return {ECS::ComponentType::CPP, component};
  } else {
    // Basic Lua component
    if (component_registry.find(name) == component_registry.end()) {
      std::cout << "error: failed to locate component " << name;
      std::exit(0);
    }
    luabridge::LuaRef component = luabridge::newTable(lua_state);
    establish_inheritance(component, component_registry.at(name));
    component["key"] = key;
    component["type"] = name;
    if (component["enabled"].isNil())
      component["enabled"] = true;
    return {ECS::ComponentType::LUA, component};
  }
}

void ECS::Lua_Log(const std::string& message) {
  std::cout << message << std::endl;
}

void ECS::Lua_LogError(const std::string& message) {
  std::cerr << message << std::endl;
}
int ECS::Lua_App_GetFrame() {
  return Helper::GetFrameNumber();
}
void ECS::Lua_App_Sleep(const int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
void ECS::Lua_App_Quit() {
  exit(0);
}
void ECS::Lua_App_OpenURL(const std::string& url) {
#if defined(_WIN32)  // Windows
  std::string command = "start " + url;
#elif defined(__APPLE__)  // macOS
  std::string command = "open " + url;
#else                     // Assuming Linux/Unix for others
  std::string command = "xdg-open " + url;
#endif

  std::system(command.c_str());
}
}  // namespace App