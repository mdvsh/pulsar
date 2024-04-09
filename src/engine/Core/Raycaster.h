//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_RAYCASTER_H_
#define PULSAR_SRC_ENGINE_CORE_RAYCASTER_H_


#include <box2d/box2d.h>

// clang-format off
#include "lua.hpp"
#include <LuaBridge/LuaBridge.h>
// clang-format on

#include "RaycastResult.h"
#include "SceneManager.h"

#include <vector>

class SingleRaycastCallback : public b2RayCastCallback {
 public:
  SingleRaycastCallback() = default;

  float ReportFixture(b2Fixture* fixture,
                      const b2Vec2& point,
                      const b2Vec2& normal,
                      float fraction) override;

  bool hit = false;
  RaycastResult result;
};

class MultipleRaycastCallback : public b2RayCastCallback {
 public:
  MultipleRaycastCallback() = default;

  float ReportFixture(b2Fixture* fixture,
                      const b2Vec2& point,
                      const b2Vec2& normal,
                      float fraction) override;
  void sort_results();

  std::vector<RaycastResult> results;
};

class Raycaster {
 public:
  static luabridge::LuaRef LuaRaycast(const b2Vec2& pos,
                                      const b2Vec2& dir,
                                      float distance);
  static luabridge::LuaRef LuaRaycastAll(const b2Vec2& pos,
                                         const b2Vec2& dir,
                                         float distance);
  static luabridge::LuaRef generate_hit_result_table(
      const std::vector<RaycastResult>& results);
  [[maybe_unused]] static std::vector<luabridge::LuaRef> multiple_raycast_results;
};


#endif  // PULSAR_SRC_ENGINE_CORE_RAYCASTER_H_
