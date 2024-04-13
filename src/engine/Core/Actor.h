//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_ACTOR_H_
#define PULSAR_SRC_ENGINE_CORE_ACTOR_H_


#include <rapidjson/document.h>
#include <iostream>
#include <string>

#include <SDL2/SDL.h>
#include <cmath>
#include <optional>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include "glm/glm.hpp"

#include "EngineUtils.h"

// clang-format off
#include "lua.hpp"
#include <LuaBridge/LuaBridge.h>
// clang-format on

inline size_t g_uuid = 0;
inline size_t g_component_id = 0;

typedef std::map<std::string, luabridge::LuaRef> entity_component_map;

enum class ComponentType { DEFAULT [[maybe_unused]], JIT [[maybe_unused]] };

using key_flag_pair = std::pair<std::string, ComponentType>;
struct KeyFlagComparator {
  bool operator()(const key_flag_pair& lhs, const key_flag_pair& rhs) const {
    return lhs.first < rhs.first;
  }
};

typedef std::vector<luabridge::LuaRef> component_list;

class Actor {
 public:
  entity_component_map entity_components;
  std::vector<std::string> entity_component_properties;
  std::unordered_map<std::string, std::set<std::string>>
      entity_components_by_type;
  std::unordered_map<EngineUtils::LifeCycle,
                     std::set<std::pair<std::string, luabridge::LuaRef>>>
      lifecycle_function_map;
  std::set<std::string> entity_on_start_component_keys;
  //  std::set<std::string> entity_on_destroy_component_keys;

  // std::set<std::pair<std::string, luabridge::LuaRef>, EngineUtils::kv_hash>
  // entity_on_start_components;
  entity_component_map entity_JIT_added_components;
  std::unordered_set<std::string> entity_JIT_removed_components;

  std::string name;
  size_t _id;

  // Need to do it disjointly from empty constructor so doesn't get called twice
  // in loading from sparse file. Find better way later...
  void set_id() { _id = ++g_uuid; }

  friend std::ostream& operator<<(std::ostream& os, const Actor& actor) {
    os << "Actor: " << actor.name << " | ID: " << actor._id << std::endl;
    return os;
  }

  ~Actor() = default;

  [[nodiscard]] std::string GetName() const { return name; }

  [[nodiscard]] size_t GetID() const { return _id; }

  // remove consts in these APIs as the user should be able to get the comp and
  // change it however they want previously also did it but pragmatically should
  // allow access from C++ interface too.
  [[nodiscard]] luabridge::LuaRef GetComponentByKey(const std::string& key);
  [[nodiscard]] luabridge::LuaRef GetComponent(const std::string& type);
  [[nodiscard]] luabridge::LuaRef GetComponents(const std::string& type);

  static void LuaOnStart(Actor* actor);
  luabridge::LuaRef LuaAddComponent(const std::string& type);
  void LuaRemoveComponent(const luabridge::LuaRef& component);
  static luabridge::LuaRef LuaCreateActor(const std::string& template_name);
  static void LuaDestroyActor(Actor* victim);
  [[maybe_unused]] void DebugPrint() const;

  [[maybe_unused]] component_list InternalGetComponents(const std::string& type);
  void populate_lifecycle_functions(const luabridge::LuaRef&,
                                    const std::string&);

};

struct ColliderCacheHash {
  size_t operator()(const std::pair<Actor*, Actor*>& pair) const {
    auto ptr1 = reinterpret_cast<uintptr_t>(pair.first);
    auto ptr2 = reinterpret_cast<uintptr_t>(pair.second);

    // A simple symmetric combination of the two pointer hashes, they're always
    // minmaxed in order.
    size_t hash1 = std::hash<uintptr_t>{}(ptr1);
    size_t hash2 = std::hash<uintptr_t>{}(ptr2);

    // Combine the two hashes inspired by  the 'boost::hash_combine' method
    return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2));
  }
};

#endif  // PULSAR_SRC_ENGINE_CORE_ACTOR_H_
