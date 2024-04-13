//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_CONTACTLISTENER_H_
#define PULSAR_SRC_ENGINE_CORE_CONTACTLISTENER_H_

#include <box2d/box2d.h>
#include <string>

#include "Actor.h"
#include "Contact.h"
#include "Renderer.h"  // for static log_error

class Rigidbody;

class ContactListener : public b2ContactListener {
 public:
  const std::string CONTACT_TAG = "CollisionResponder";

  void BeginContact(b2Contact* contact) override;
  void EndContact(b2Contact* contact) override;

  //  void LuaOnContactHandle(const Contact&, Contact::Event, Actor*);
  static void LuaOnContactHandle(const Contact& contact,
                                 EngineUtils::LifeCycle event,
                                 Actor* caller);
};


#endif  // PULSAR_SRC_ENGINE_CORE_CONTACTLISTENER_H_
