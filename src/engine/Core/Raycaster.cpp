//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#include "Raycaster.h"
#include "ECS.h"

[[maybe_unused]] std::vector<luabridge::LuaRef> Raycaster::multiple_raycast_results;

luabridge::LuaRef Raycaster::LuaRaycast(const b2Vec2& pos,
                                        const b2Vec2& dir,
                                        float distance) {
  const auto L = App::ECS::getInstance().get_lua_state();
  if (distance <= 0.0f)
    return {L};

  const auto phys_world = SceneManager::getInstance().GetPhysWorld();
  SingleRaycastCallback callback;
  const auto& scaled_dir = b2Vec2(dir.x * distance, dir.y * distance);
  phys_world->RayCast(&callback, pos, pos + scaled_dir);

  if (callback.hit)
    return {L, callback.result};
  return {L};
}

luabridge::LuaRef Raycaster::LuaRaycastAll(const b2Vec2& pos,
                                           const b2Vec2& dir,
                                           float distance) {
  const auto L = App::ECS::getInstance().get_lua_state();
  if (distance <= 0.0f)
    return {L};

  const auto phys_world = SceneManager::getInstance().GetPhysWorld();
  MultipleRaycastCallback callback;

  const auto& scaled_dir = b2Vec2(dir.x * distance, dir.y * distance);
  phys_world->RayCast(&callback, pos, pos + scaled_dir);
  callback.sort_results();
  return {L, generate_hit_result_table(callback.results)};
}

luabridge::LuaRef Raycaster::generate_hit_result_table(
    const std::vector<RaycastResult>& results) {
  const auto L = App::ECS::getInstance().get_lua_state();
  luabridge::LuaRef hit_results_table = luabridge::newTable(L);
  int i = 1;
  for (const auto& raycast_result : results) {
    hit_results_table[i++] = luabridge::LuaRef(L, raycast_result);
  }
  return hit_results_table;
}

float SingleRaycastCallback::ReportFixture(b2Fixture* fixture,
                                           const b2Vec2& point,
                                           const b2Vec2& normal,
                                           float fraction) {
  if (fraction == 0.0f)
    return -1.0f;
  if (EngineUtils::is_phantom_fixture(fixture))
    return -1.0f;

  if (fixture->GetUserData().pointer) {
    result.SetActor(reinterpret_cast<Actor*>(fixture->GetUserData().pointer));
    result.SetPoint(point);
    result.SetNormal(normal);
    result.SetTrigger(fixture->IsSensor());
    hit = true;
    return fraction;
  }

  return -1.0f;
}
float MultipleRaycastCallback::ReportFixture(b2Fixture* fixture,
                                             const b2Vec2& point,
                                             const b2Vec2& normal,
                                             float fraction) {
  if (fraction == 0.0f)
    return -1.0f;

  if (EngineUtils::is_phantom_fixture(fixture))
    return -1.0f;

  if (fixture->GetUserData().pointer) {
    RaycastResult result;
    result.SetActor(reinterpret_cast<Actor*>(fixture->GetUserData().pointer));
    result.SetPoint(point);
    result.SetNormal(normal);
    result.SetTrigger(fixture->IsSensor());
    results.push_back(result);
  }

  return 1.0f;
}
void MultipleRaycastCallback::sort_results() {
  std::sort(results.begin(), results.end(),
            [](const RaycastResult& a, const RaycastResult& b) {
              return a.GetPoint().Length() < b.GetPoint().Length();
            });
}
