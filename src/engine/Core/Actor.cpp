//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#include "Actor.h"
#include "ActorTemplate.h"
#include "ECS.h"
#include "Renderer.h"
#include "SceneManager.h"

void Actor::LuaOnStart(Actor* actor) {
  for (const auto& [key, component] : actor->entity_components) {
    // inject convenient self reference
    component["actor"] = actor;
    try {
      const auto enabled = component["enabled"].cast<bool>();
      const auto& on_start = component["OnStart"];
      if (enabled and on_start.isFunction()) {
        actor->entity_on_start_component_keys.insert(key);
      }
    } catch (const luabridge::LuaException& e) {
      Renderer::log_error(actor->name, e);
    }
  }
}

luabridge::LuaRef Actor::LuaAddComponent(const std::string& type) {
  const std::string key = "r" + std::to_string(g_component_id++);

  auto new_comp_pair = App::ECS::getInstance().create_component(key, type);
  auto component = new_comp_pair.second;
  const auto& scm = SceneManager::getInstance();

  Actor* actor = this;
  if (scm.jit_created_actors_map.count(_id) > 0)
    actor = scm.jit_created_actors_map.at(_id);

  // TODO: use later
  actor->populate_lifecycle_functions(component, key);

  if (new_comp_pair.first == App::ECS::ComponentType::LUA) {
    if (const auto& on_start = component["OnStart"]; on_start.isFunction())
      actor->entity_on_start_component_keys.insert(key);
  } else {
    actor->entity_on_start_component_keys.insert(key);
  }
  component["actor"] = actor;
  // collect step
  actor->entity_JIT_added_components.emplace(key, component);
  actor->entity_components_by_type[type].insert(key);

  return component;
}

void Actor::LuaRemoveComponent(const luabridge::LuaRef& component) {
  const auto& scm = SceneManager::getInstance();
  Actor* actor = this;
  if (scm.jit_created_actors_map.count(_id) > 0)
    actor = scm.jit_created_actors_map.at(_id);

  component["enabled"] = false;
  const auto& key = component["key"].cast<std::string>();
  if (actor->entity_components.find(key) != actor->entity_components.end()) {
    actor->entity_JIT_removed_components.insert(key);
    const auto& name_to_remove = component["type"].cast<std::string>();
    actor->entity_components_by_type[name_to_remove].erase(key);
  }
}

luabridge::LuaRef Actor::LuaCreateActor(const std::string& template_name) {
  const auto L = App::ECS::getInstance().get_lua_state();
  if (template_name.empty())
    return {L};

  // move to helper later
  const std::string templates_path = "resources/actor_templates";
  const std::string actor_templates_path =
      templates_path + "/" + template_name + ".template";

  if (not std::filesystem::exists(actor_templates_path)) {
    std::cout << "error: template " << template_name << " is missing";
    std::exit(0);
  }
  rapidjson::Document actor_templates;
  EngineUtils::ReadJsonFile(actor_templates_path, actor_templates);

  auto* new_actor = new Actor;
  ActorTemplate::update_from_source_file(*new_actor, actor_templates);
  new_actor->set_id();

  auto& scm = SceneManager::getInstance();
  //    scm.source_of_jit_instantiated_actors.emplace_back(std::move(new_actor));
  //    const auto actor_ptr = &scm.source_of_jit_instantiated_actors.back();
  scm.jit_created_actors_map[new_actor->_id] = new_actor;
  scm.jit_instantiated_actors.push_back(new_actor);
  scm.actors_by_name[new_actor->name].push_back(new_actor);
  Actor::LuaOnStart(new_actor);
  return {L, *new_actor};
}

void Actor::LuaDestroyActor(Actor* victim) {
  if (not victim)
    return;
  auto& scm = SceneManager::getInstance();
  if (scm.jit_created_actors_map.count(victim->_id) > 0)
    victim = scm.jit_created_actors_map.at(victim->_id);

  // delete from actors by name so not accessible by find, findall
  for (auto& [name, actors] : scm.actors_by_name) {
    // Find the victim actor in the vector by matching the ID
    auto it = std::find_if(
        actors.begin(), actors.end(),
        [victim](const Actor* actor) { return actor->_id == victim->_id; });
    // If found, remove the actor from the vector efficiently
    if (it != actors.end()) {
      std::swap(*it, actors.back());
      actors.pop_back();

      if (actors.empty()) {
        scm.actors_by_name.erase(name);
      }
      break;
      // Assuming an actor can only be in one vector, we can break after removal
    }
  }

  victim->name.clear();
  for (auto& [k, component] : victim->entity_components)
    component["enabled"] = false;
  scm.victim_actors_this_frame.emplace_back(victim->_id, victim);
}

// TODO: refactor for code duplication removal later

luabridge::LuaRef Actor::GetComponentByKey(const std::string& key) {
  const auto L = App::ECS::getInstance().get_lua_state();
  if (const auto recently_deleted = entity_JIT_removed_components.find(key);
      recently_deleted != entity_JIT_removed_components.end()) {
    return {L};
  }
  const auto& scm = SceneManager::getInstance();
  Actor* actor = this;
  if (scm.jit_created_actors_map.count(_id) > 0)
    actor = scm.jit_created_actors_map.at(_id);
  if (const auto component = actor->entity_components.find(key);
      component != actor->entity_components.end()) {
    return {L, component->second};
  }
  return {L};
}
luabridge::LuaRef Actor::GetComponent(const std::string& type) {
  const auto L = App::ECS::getInstance().get_lua_state();
  const auto& scm = SceneManager::getInstance();
  Actor* actor = this;
  if (scm.jit_created_actors_map.count(_id) > 0)
    actor = scm.jit_created_actors_map.at(_id);
  if (const auto key_iter = actor->entity_components_by_type.find(type);
      key_iter != actor->entity_components_by_type.end()) {
    const auto key = *key_iter->second.begin();
    if (const auto recently_deleted =
            actor->entity_JIT_removed_components.find(key);
        recently_deleted != actor->entity_JIT_removed_components.end()) {
      return {L};
    }
    if (const auto component = actor->entity_components.find(key);
        component != actor->entity_components.end()) {
      return {L, component->second};
    }
  }
  return {L};
}
luabridge::LuaRef Actor::GetComponents(const std::string& type) {
  const auto L = App::ECS::getInstance().get_lua_state();
  luabridge::LuaRef components_table = luabridge::newTable(L);

  const auto& scm = SceneManager::getInstance();
  Actor* actor = this;
  if (scm.jit_created_actors_map.count(_id) > 0)
    actor = scm.jit_created_actors_map.at(_id);
  if (const auto component = actor->entity_components_by_type.find(type);
      component != actor->entity_components_by_type.end()) {
    int i = 1;
    for (const auto& component_key : component->second) {
      const auto exists =
          actor->entity_JIT_removed_components.find(component_key) ==
              actor->entity_JIT_removed_components.end() and
          actor->entity_components.find(component_key) !=
              actor->entity_components.end();
      if (exists) {
        components_table[i++] =
            luabridge::LuaRef(L, actor->entity_components.at(component_key));
      }
    }
  }
  return components_table;
}

component_list Actor::InternalGetComponents(const std::string& type) {
  component_list query_result;
  const auto L = App::ECS::getInstance().get_lua_state();
  const auto& scm = SceneManager::getInstance();

  Actor* actor = this;
  if (scm.jit_created_actors_map.count(_id) > 0)
    actor = scm.jit_created_actors_map.at(_id);

  if (const auto component = actor->entity_components_by_type.find(type);
      component != actor->entity_components_by_type.end()) {
    for (const auto& component_key : component->second) {
      const auto exists =
          actor->entity_JIT_removed_components.find(component_key) ==
              actor->entity_JIT_removed_components.end() and
          actor->entity_components.find(component_key) !=
              actor->entity_components.end();
      if (exists) {
        query_result.emplace_back(L,
                                  actor->entity_components.at(component_key));
      }
    }
  }
  return query_result;
}

[[maybe_unused]] void Actor::DebugPrint() const {
  std::cout << "Actor: " << name << std::endl;
  for (const auto& [key, component] : entity_components) {
    std::cout << "Component: " << key << std::endl;
  }
  for (const auto& [type, keys] : entity_components_by_type) {
    std::cout << "Type: " << type << " | Count: " << keys.size() << std::endl;
  }
}

void Actor::populate_lifecycle_functions(const luabridge::LuaRef& component,
                                         const std::string& key) {
  if (const auto& on_start = component["OnStart"]; on_start.isFunction()) {
    lifecycle_function_map[EngineUtils::LifeCycle::OnStart].emplace(key,
                                                                    on_start);
  }
  if (const auto& on_update = component["OnUpdate"]; on_update.isFunction()) {
    lifecycle_function_map[EngineUtils::LifeCycle::OnUpdate].emplace(key,
                                                                     on_update);
  }
  if (const auto& on_end = component["OnLateUpdate"]; on_end.isFunction()) {
    lifecycle_function_map[EngineUtils::LifeCycle::OnLateUpdate].emplace(
        key, on_end);
  }
  if (const auto& on_destroy = component["OnDestroy"];
      on_destroy.isFunction()) {
    lifecycle_function_map[EngineUtils::LifeCycle::OnDestroy].emplace(
        key, on_destroy);
  }
  if (const auto& on_collision_enter = component["OnCollisionEnter"];
      on_collision_enter.isFunction()) {
    lifecycle_function_map[EngineUtils::LifeCycle::OnCollisionEnter].emplace(
        key, on_collision_enter);
  }
  if (const auto& on_collision_exit = component["OnCollisionExit"];
      on_collision_exit.isFunction()) {
    lifecycle_function_map[EngineUtils::LifeCycle::OnCollisionExit].emplace(
        key, on_collision_exit);
  }
  if (const auto& on_trigger_enter = component["OnTriggerEnter"];
      on_trigger_enter.isFunction()) {
    lifecycle_function_map[EngineUtils::LifeCycle::OnTriggerEnter].emplace(
        key, on_trigger_enter);
  }
  if (const auto& on_trigger_exit = component["OnTriggerExit"];
      on_trigger_exit.isFunction()) {
    lifecycle_function_map[EngineUtils::LifeCycle::OnTriggerExit].emplace(
        key, on_trigger_exit);
  }
}
