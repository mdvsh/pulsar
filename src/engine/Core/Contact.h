//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_CONTACT_H_
#define PULSAR_SRC_ENGINE_CORE_CONTACT_H_

#include <box2d/box2d.h>
#include <iostream>
#include <string>

class Actor;

class Contact {
 public:
  Contact()
      : other(nullptr),
        point(0.0f, 0.0f),
        relative_velocity(0.0f, 0.0f),
        normal(0.0f, 0.0f){};
  ~Contact() = default;

  enum class Event {
    OnCollisionEnter,
    OnCollisionExit,
    OnTriggerEnter,
    OnTriggerExit
  };

  static std::string GetEventString(Event event) {
    switch (event) {
      case Event::OnCollisionEnter:
        return "OnCollisionEnter";
      case Event::OnCollisionExit:
        return "OnCollisionExit";
      case Event::OnTriggerEnter:
        return "OnTriggerEnter";
      case Event::OnTriggerExit:
        return "OnTriggerExit";

      default:
        std::cout << "[FATAL] Unknown event type in "
                     "ContactListener::LuaOnContactHandle"
                  << std::endl;
        return "Unknown";
    }
  }

  [[nodiscard]] Actor* GetOther() const { return other; }
  void SetOther(Actor* actor) { other = actor; }

  [[nodiscard]] b2Vec2 GetPoint() const { return point; }
  void SetPoint(const b2Vec2& p) { point = p; }

  [[nodiscard]] b2Vec2 GetRelativeVelocity() const { return relative_velocity; }
  void SetRelativeVelocity(const b2Vec2& v) { relative_velocity = v; }

  [[nodiscard]] b2Vec2 GetNormal() const { return normal; }
  void SetNormal(const b2Vec2& n) { normal = n; }

  void SetTriggerContact() {
    point = b2Vec2(-999.0f, -999.0f);
    normal = b2Vec2(-999.0f, -999.0f);
  }

  static Actor* GetActorA(b2Contact* contact) {
    return reinterpret_cast<Actor*>(
        contact->GetFixtureA()->GetUserData().pointer);
  }
  static Actor* GetActorB(b2Contact* contact) {
    return reinterpret_cast<Actor*>(
        contact->GetFixtureB()->GetUserData().pointer);
  }

 private:
  Actor* other;              // other actor involved in the contact
  b2Vec2 point;              // first point of collision
  b2Vec2 relative_velocity;  // body_a velocity - body_b velocity
  b2Vec2 normal;  // unit vector of collision force, perpendicular to surface of
  // collision
};


#endif  // PULSAR_SRC_ENGINE_CORE_CONTACT_H_
