//
// Created by Madhav Shekhar Sharma on 1/30/24.
//

#include "SceneManager.h"

#include <rapidjson/document.h>
#include <algorithm>
#include <cstdlib>
#include <filesystem>

#include "ActorTemplate.h"
#include "ECS.h"
#include "EngineUtils.h"
#include "Resources.hpp"

std::optional<std::string> SceneManager::latest_scene_change_request =
    std::nullopt;

void SceneManager::initialize(const rapidjson::Document& game_config) {
  const std::string initial_scene =
      EngineUtils::LoadStringFromJson(game_config, "initial_scene");
  if (initial_scene.empty()) {
    std::cout << "error: initial_scene unspecified";
    std::exit(0);
  }

  auto resources_path = App::Resources::game_path();
  auto lua_components_path = resources_path / "component_types";

  if (std::filesystem::exists(lua_components_path)) {
    auto& ecs = App::ECS::getInstance();
    ecs.initialize();
  }

  std::string scene_path = initial_scene + ".scene";
  auto scene_file = resources_path / "scenes" / scene_path;
  if (not std::filesystem::exists(scene_file)) {
    std::cout << "error: scene " << initial_scene << " is missing";
    std::exit(0);
  }

  rapidjson::Document initial_scene_data;
  EngineUtils::ReadJsonFile(scene_file, initial_scene_data);
  load_scene_actors(initial_scene_data);
  current_scene_name = initial_scene;
}

void SceneManager::trigger_scene_change(const std::string& scene_name) {
  const std::string scene_path = "resources/scenes/" + scene_name + ".scene";
  if (not std::filesystem::exists(scene_path)) {
    std::cout << "error: scene " << scene_name << " is missing";
    std::exit(0);
  }
  rapidjson::Document new_scene_data;
  EngineUtils::ReadJsonFile(scene_path, new_scene_data);

  for (const auto& actor : scene_actors) {
    if (ids_of_scene_persisting_actors.count(actor._id) <= 0 and
        not actor.entity_on_start_component_keys.empty()) {
      for (const auto& [key, component] : actor.entity_components) {
        try {
          if (luabridge::LuaRef on_destroy = component["OnDestroy"];
              on_destroy.isFunction()) {
            on_destroy(component);
          }
        } catch (const luabridge::LuaException& e) {
          Renderer::log_error(actor.name, e);
        }
      }
    }
  }

  scene_actors.clear();
  copy_of_scene_actors.clear();
  // safety ?
  actors_by_name.clear();
  actor_booted_component_keys.clear();

  // Recall: this is shallow copy, value (pointer) access wouldn't work,
  // although we just need key/count.
  auto copy_of_jit_actors = jit_created_actors_map;
  jit_created_actors_map.clear();

  if (not source_of_scene_persisting_actors.empty()) {
    for (const auto& actor_to_persist : source_of_scene_persisting_actors) {
      scene_actors.push_back(actor_to_persist);
      copy_of_scene_actors.push_back(&scene_actors.back());
      actors_by_name[actor_to_persist.name].push_back(&scene_actors.back());
      // re-set JIT created actors map in new scene
      if (const size_t jit_id = actor_to_persist._id;
          copy_of_jit_actors.count(jit_id) > 0)
        jit_created_actors_map[jit_id] = &scene_actors.back();
    }
    //		source_of_scene_persisting_actors.clear();
  }
  load_scene_actors(new_scene_data);
  current_scene_name = scene_name;
}

void SceneManager::load_scene_actors(const rapidjson::Document& scene_data) {
  if (not scene_data.HasMember("actors") or
      not scene_data["actors"].IsArray()) {
    return;
  }
  const auto& actors = scene_data["actors"].GetArray();
  scene_actors.reserve(actors.Size());
  copy_of_scene_actors.reserve(actors.Size());

  for (const auto& actorData : actors) {
    Actor actor;
    if (actorData.HasMember("template")) {
      if (const std::string template_name = actorData["template"].GetString();
          actor_templates_map.count(template_name) > 0) {
        actor = actor_templates_map.at(template_name);
        actor.set_id();
      } else {
        actor = ActorTemplate::load_actor_template(template_name);
        actor_templates_map.emplace(template_name, actor);
      }
    } else {
      actor.set_id();
    }
    // override template
    ActorTemplate::update_from_source_file(actor, actorData);
    // if (actor.can_render()) {
    //	// get_image_dimensions, also invokes get_or_create_texture.
    //	auto [w, h] = renderer->get_image_dimensions(actor.sprite_name);
    //	actor.set_default_pivot_offset_and_texture_dims(w, h);
    // }

    scene_actors.emplace_back(std::move(actor));
    const auto actor_ptr = &scene_actors.back();
    if (actor_ptr) {
      copy_of_scene_actors.push_back(actor_ptr);
      actors_by_name[actor_ptr->name].push_back(actor_ptr);
      Actor::LuaOnStart(actor_ptr);
    } else {
      std::cerr << "[FATAL] actor_ptr shouldn't have been null\n";
    }
  }
}

void SceneManager::update_scene_actors() {
  for (const auto& actor : copy_of_scene_actors) {
    if (not actor->entity_JIT_added_components.empty()) {
      // this cycle of JIT comps added by OnUpdate & OnLateUpdate
      actor->entity_components.insert(
          actor->entity_JIT_added_components.begin(),
          actor->entity_JIT_added_components.end());
      actor->entity_JIT_added_components.clear();
    }

    if (not actor->entity_on_start_component_keys.empty() and
        ids_of_scene_persisting_actors.count(actor->_id) == 0) {
      for (auto& key : actor->entity_on_start_component_keys) {
        // Need to do a .find as component might be instantiated JIT, in which
        // case we don't OnStart until next frame.
        if (const auto component_itr = actor->entity_components.find(key);
            component_itr != actor->entity_components.end()) {
          const auto& component = component_itr->second;
          const bool enabled = component["enabled"].cast<bool>();
          try {
            if (luabridge::LuaRef on_start = component["OnStart"]; enabled) {
              on_start(component);
            }
          } catch (const luabridge::LuaException& e) {
            Renderer::log_error(actor->name, e);
          }
          actor_booted_component_keys.push_back(key);
        }
      }

      if (not actor_booted_component_keys.empty()) {
        for (auto& key : actor_booted_component_keys)
          actor->entity_on_start_component_keys.erase(key);

        actor_booted_component_keys.clear();
      }
    }
  }

  for (const auto& actor : copy_of_scene_actors) {
    const bool has_JIT_added_components =
        (!actor->entity_JIT_added_components.empty());
    // this cycle of JIT comps added by OnStart
    if (not actor->entity_JIT_added_components.empty()) {
      actor->entity_components.insert(
          actor->entity_JIT_added_components.begin(),
          actor->entity_JIT_added_components.end());
    }
    // can OnStart just delete components ? then need to collect and alter
    // before iterating over entity components

    // OnUpdate
    for (const auto& [key, component] : actor->entity_components) {
      bool just_added_component_for_key = false;
      if (has_JIT_added_components)
        just_added_component_for_key =
            actor->entity_JIT_added_components.count(key) > 0;
      try {
        const auto& on_update = component["OnUpdate"];
        bool component_enabled = component["enabled"].cast<bool>();
        if (component_enabled and not just_added_component_for_key and
            on_update.isFunction()) {
          on_update(component);
        }
      } catch (const luabridge::LuaException& e) {
        Renderer::log_error(actor->name, e);
      }

      if (luabridge::LuaRef on_late_update = component["OnLateUpdate"];
          not just_added_component_for_key and on_late_update.isFunction())
        late_update_components.emplace_back(actor, component);
    }
    if (not actor->entity_JIT_removed_components.empty()) {
      for (const auto& to_be_removed_key :
           actor->entity_JIT_removed_components) {
        to_be_removed_actor_components.emplace_back(actor, to_be_removed_key);
      }
      actor->entity_JIT_removed_components.clear();
    }
  }

  for (const auto& [actor, component] : late_update_components) {
    try {
      bool component_enabled = component["enabled"].cast<bool>();
      if (luabridge::LuaRef on_late_update = component["OnLateUpdate"];
          component_enabled)
        on_late_update(component);
    } catch (const luabridge::LuaException& e) {
      Renderer::log_error(actor->name, e);
    }
  }

  for (const auto& [actor, key] : to_be_removed_actor_components) {
    const auto& component = actor->entity_components.at(key);
    try {
      if (luabridge::LuaRef on_destroy = component["OnDestroy"];
          on_destroy.isFunction()) {
        on_destroy(component);
      }
    } catch (const luabridge::LuaException& e) {
      Renderer::log_error(actor->name, e);
    }
    actor->entity_components.erase(key);
  }

  if (not victim_actors_this_frame.empty()) {
    for (auto& victim_pair : victim_actors_this_frame) {
      const auto actor_found_at =
          std::find_if(copy_of_scene_actors.begin(), copy_of_scene_actors.end(),
                       [&victim_pair](const Actor* actor) {
                         return actor->_id == victim_pair.first;
                       });
      if (actor_found_at != copy_of_scene_actors.end()) {
        for (const auto& [key, component] :
             (*actor_found_at)->entity_components) {
          try {
            if (luabridge::LuaRef on_destroy = component["OnDestroy"];
                on_destroy.isFunction()) {
              on_destroy(component);
            }
          } catch (const luabridge::LuaException& e) {
            Renderer::log_error((*actor_found_at)->name, e);
          }
        }
        copy_of_scene_actors.erase(actor_found_at);
        if (jit_created_actors_map.count(victim_pair.first) > 0 and
            victim_pair.second)
          delete victim_pair.second;
      }
    }
  }

  for (const auto& jit_actor_ptr : jit_instantiated_actors) {
    copy_of_scene_actors.push_back(jit_actor_ptr);
  }

  late_update_components.clear();
  to_be_removed_actor_components.clear();
  jit_instantiated_actors.clear();
}

[[maybe_unused]] void SceneManager::sort_actors_by_uuid(std::vector<Actor*>& actors) {
  std::sort(actors.begin(), actors.end(),
            [](const Actor* a, const Actor* b) { return a->_id < b->_id; });
}
luabridge::LuaRef SceneManager::GetActor(const std::string& name) {
  const auto L = App::ECS::getInstance().get_lua_state();
  const auto actors_by_name = SceneManager::getInstance().actors_by_name;
  if (const auto actor_itr = actors_by_name.find(name);
      actor_itr != actors_by_name.end()) {
    return {L, *(actor_itr->second[0])};
  }
  return {L};
}
luabridge::LuaRef SceneManager::GetActors(const std::string& name) {
  const auto L = App::ECS::getInstance().get_lua_state();
  const auto actors_by_name = SceneManager::getInstance().actors_by_name;
  luabridge::LuaRef actors_table = luabridge::newTable(L);
  if (const auto actors_itr = actors_by_name.find(name);
      actors_itr != actors_by_name.end()) {
    int i = 1;
    for (const auto& actor : actors_itr->second) {
      actors_table[i++] = luabridge::LuaRef(L, *actor);
    }
  }
  return actors_table;
}
void SceneManager::LuaLoadNewScene(const std::string& name) {
  latest_scene_change_request = name;
}
std::string SceneManager::LuaGetCurrentScene() {
  return getInstance().current_scene_name;
}
void SceneManager::LuaPersistActor(const Actor* actor_to_persist) {
  auto& scm = SceneManager::getInstance();
  const auto& all_actors = scm.actors_by_name;
  auto& source_to_persist = scm.source_of_scene_persisting_actors;
  auto& ids_to_persist = scm.ids_of_scene_persisting_actors;

  if (const std::string& name = actor_to_persist->name;
      all_actors.find(name) != all_actors.end()) {
    const auto& actors_with_this_name = all_actors.at(name);
    for (const auto& actor_ptr : actors_with_this_name) {
      if (actor_ptr->_id == actor_to_persist->_id) {
        if (actor_ptr) {
          source_to_persist.push_back(*actor_ptr);
          ids_to_persist.insert(actor_ptr->_id);
        }
        break;
      }
    }
  }
}

b2World* SceneManager::GetPhysWorld() const {
  if (not phys_world_initialized) {
    std::cerr << "[FATAL] phys world NOT initialized but accessed.\n";
    std::exit(0);
  }
  return phys_world;
}

void SceneManager::CreatePhysWorld() {
  if (not phys_world_initialized) {
    phys_world = new b2World(b2Vec2(0.0f, 9.8f));
    auto* contact_listener = new ContactListener();
    phys_world->SetContactListener(contact_listener);
    phys_world_initialized = true;
  }
}

void SceneManager::StepPhysWorld() const {
  //  if (not phys_world_initialized) {
  //	std::cerr << "[FATAL] phys world NOT initialized but updated.\n";
  //	std::exit(0);
  //  }
  if (phys_world_initialized)
    phys_world->Step(1.0f / 60.0f, 8, 3);
}
