//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_RAYCASTRESULT_H_
#define PULSAR_SRC_ENGINE_CORE_RAYCASTRESULT_H_


#include <box2d/box2d.h>

#include "Actor.h"

class RaycastResult {
 public:
  [[nodiscard]] Actor* GetActor() const { return actor; }
  void SetActor(Actor* a) { actor = a; }

  [[nodiscard]] b2Vec2 GetPoint() const { return point; }
  void SetPoint(const b2Vec2& p) { point = p; }

  [[nodiscard]] b2Vec2 GetNormal() const { return normal; }
  void SetNormal(const b2Vec2& n) { normal = n; }

  [[nodiscard]] bool IsTrigger() const { return is_trigger; }
  void SetTrigger(bool t) { is_trigger = t; }

  RaycastResult()
      : actor(nullptr),
        point(0.0f, 0.0f),
        normal(0.0f, 0.0f),
        is_trigger(false){};
  ~RaycastResult() = default;

  void reset() {
    actor = nullptr;
    point = b2Vec2(0.0f, 0.0f);
    normal = b2Vec2(0.0f, 0.0f);
    is_trigger = false;
  }

 private:
  Actor* actor;
  b2Vec2 point;
  b2Vec2 normal;
  bool is_trigger;
};

#endif  // PULSAR_SRC_ENGINE_CORE_RAYCASTRESULT_H_
