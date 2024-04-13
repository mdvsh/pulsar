//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_RIGIDBODY_H_
#define PULSAR_SRC_ENGINE_CORE_RIGIDBODY_H_

#include <box2d/box2d.h>
#include <string>

#include "Actor.h"
#include "SceneManager.h"

class Rigidbody {
 public:
  Rigidbody();
  ~Rigidbody();

  void Initialize();
  void Destroy();
  void SetPosition(const b2Vec2& pos);
  void SetRotation(float new_rotation);
  [[nodiscard]] b2Vec2 GetPosition() const;
  [[nodiscard]] float GetRotation() const;

  void CreateCollider();
  void CreateTrigger();
  void CreatePhantomFixture();

  void AddForce(const b2Vec2& force);
  void SetVelocity(const b2Vec2& velocity);
  void SetAngularVelocity(float degrees_clockwise);
  void SetGravityScale(float scale);
  void SetUpDirection(const b2Vec2& direction);
  void SetRightDirection(const b2Vec2& direction);
  [[nodiscard]] b2Vec2 GetVelocity() const;
  [[nodiscard]] float GetAngularVelocity() const;
  [[nodiscard]] float GetGravityScale() const;
  [[nodiscard]] b2Vec2 GetUpDirection() const;
  [[nodiscard]] b2Vec2 GetRightDirection() const;

  [[nodiscard]] static float to_radian(float degree);
  [[nodiscard]] static float to_degree(float radian);

  float x;
  float y;
  std::string body_type;
  bool precise;
  float gravity_scale;
  float density;
  float angular_friction;
  float rotation;
  bool has_collider;
  bool has_trigger;

  // Collider defaults
  std::string collider_type = "box";
  float collider_width = 1.0f;
  float collider_height = 1.0f;
  float collider_radius = 0.5f;
  float friction = 0.3f;
  float bounciness = 0.3f;

  // Trigger defaults
  std::string trigger_type = "box";
  float trigger_width = 1.0f;
  float trigger_height = 1.0f;
  float trigger_radius = 0.5f;

  std::string type;
  std::string key;
  Actor* actor;  // TODO: make private ?
  bool enabled;
  static const uint16 CATEGORY_COLLIDER;
  static const uint16 CATEGORY_TRIGGER;
  static const uint16 CATEGORY_PHANTOM;

  friend std::ostream& operator<<(std::ostream& os,
                                  const Rigidbody& rigidBody) {
    os << "Rigidbody: " << rigidBody.key << " at (" << rigidBody.x << ", "
       << rigidBody.y << ")";
    return os;
  }

 private:
  b2Body* body;
};


#endif  // PULSAR_SRC_ENGINE_CORE_RIGIDBODY_H_
