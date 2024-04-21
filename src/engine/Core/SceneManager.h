//
// Created by Madhav Shekhar Sharma on 1/30/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_SCENEMANAGER_H_
#define PULSAR_SRC_ENGINE_CORE_SCENEMANAGER_H_

#include <box2d/box2d.h>
#include <rapidjson/document.h>
#include <algorithm>
#include <array>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <glm/glm.hpp>
#include <vector>

#include "Actor.h"
#include "AudioManager.h"
#include "ContactListener.h"
#include "ECS.h"
#include "EngineUtils.h"
#include "Renderer.h"

[[maybe_unused]] typedef std::
    unordered_map<glm::ivec2, std::vector<Actor*>, EngineUtils::hash_pair>
        spatial_grid;
[[maybe_unused]] typedef std::unordered_set<std::pair<Actor*, Actor*>, ColliderCacheHash>
    actor_interaction_cache;
[[maybe_unused]] typedef std::vector<std::pair<std::string, size_t>> dialogues_ctr;

class SceneManager {
  SceneManager() {}

 public:
  SceneManager(const SceneManager&) = delete;
  SceneManager& operator=(const SceneManager&) = delete;
  SceneManager(SceneManager&&) = delete;
  SceneManager& operator=(SceneManager&&) = delete;

  // Static method to access the singleton instance
  static SceneManager& getInstance() {
    static SceneManager instance;
    return instance;
  }

  int health = 3;
  int score = 0;
  [[maybe_unused]] static const size_t FPS = 60;

  // No more starting HW7
  // Actor *player_actor = nullptr;
  std::vector<Actor> scene_actors;
  std::vector<Actor*> copy_of_scene_actors;
  std::vector<Actor*> actors_to_add;
  std::unordered_map<std::string, std::vector<Actor*>> actors_by_name;
  std::unordered_map<std::string, Actor> actor_templates_map;
  std::unordered_set<std::string> serviced_on_start_components;

  b2World* phys_world = nullptr;
  [[maybe_unused]] ContactListener* phys_contact_listener = nullptr;
  bool phys_world_initialized = false;

  [[nodiscard]] b2World* GetPhysWorld() const;
  void CreatePhysWorld();
  void StepPhysWorld() const;

  void reset();

  void initialize(const rapidjson::Document& game_config);

  void trigger_scene_change(const std::string& scene_name);

  void load_scene_actors(const rapidjson::Document& scene_data);

  void update_scene_actors();
  [[maybe_unused]] void update_scene_actors_helper(
      Actor& actor,
      std::optional<glm::vec2> player_direction_vector = std::nullopt);

  [[maybe_unused]] static void sort_actors_by_uuid(std::vector<Actor*>& actors);

  [[maybe_unused]] void print_stats() const {
    std::cout << "health : " << health << ", score : " << score << std::endl;
  }

  [[maybe_unused]] [[nodiscard]] std::pair<int, int> get_stats() const {
    return {health, score};
  }

  [[maybe_unused]] [[nodiscard]] int get_health() const { return health; }

  [[maybe_unused]] void set_health(const int health_to_set) { SceneManager::health = health_to_set; }

  int health_down_frame = -180;

  [[maybe_unused]] [[nodiscard]] int get_health_down_frame() const { return health_down_frame; }

  [[maybe_unused]] void set_health_down_frame(int healthDownFrame) {
    health_down_frame = healthDownFrame;
  }

  [[maybe_unused]] [[nodiscard]] int get_score() const { return score; }

  [[maybe_unused]] void decrease_health() {
    // evidently we allow negative health lol...
    // if (health > 0) {
    --health;
    // }
  }

  [[maybe_unused]] void increase_score() { ++score; }

  // Functions to expose to lua via the Actor namespace
  [[nodiscard]] static luabridge::LuaRef GetActor(const std::string& name);
  [[nodiscard]] static luabridge::LuaRef GetActors(const std::string& name);

  std::string current_scene_name;
  static std::optional<std::string> latest_scene_change_request;

  static void LuaLoadNewScene(const std::string& name);
  [[nodiscard]] static std::string LuaGetCurrentScene();
  static void LuaPersistActor(const Actor*);

  actor_component_list late_update_components;
  actor_component_list on_destroy_components;
  actor_component_key_list to_be_removed_actor_components;
  std::vector<Actor*> jit_instantiated_actors;
  std::vector<std::pair<size_t, Actor*>> victim_actors_this_frame;
  std::vector<Actor> source_of_scene_persisting_actors;
  std::unordered_set<size_t> ids_of_scene_persisting_actors;
  actor_id_map jit_created_actors_map;
  std::vector<std::string> actor_booted_component_keys;
};


#endif  // PULSAR_SRC_ENGINE_CORE_SCENEMANAGER_H_
