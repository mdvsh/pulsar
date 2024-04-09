//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#include "ContactListener.h"

void ContactListener::BeginContact(b2Contact* contact) {
  Contact new_contact;
  auto A = Contact::GetActorA(contact);
  auto B = Contact::GetActorB(contact);

  if (not A or not B)
    return;

  auto fixtureA = contact->GetFixtureA();
  auto fixtureB = contact->GetFixtureB();

  if (EngineUtils::is_phantom_fixture(fixtureA) or
      EngineUtils::is_phantom_fixture(fixtureB)) {
    // The fixtures should not collide, skip the contact handling
    return;
  }

  new_contact.SetOther(B);
  new_contact.SetRelativeVelocity(fixtureA->GetBody()->GetLinearVelocity() -
                                  fixtureB->GetBody()->GetLinearVelocity());

  bool isTriggerContact = fixtureA->IsSensor() || fixtureB->IsSensor();
  if (isTriggerContact) {
    new_contact.SetTriggerContact();
  } else {
    b2WorldManifold worldManifold;
    contact->GetWorldManifold(&worldManifold);
    new_contact.SetPoint(worldManifold.points[0]);
    new_contact.SetNormal(worldManifold.normal);
  }
  if (fixtureA->IsSensor() and fixtureB->IsSensor()) {
    LuaOnContactHandle(const_cast<const Contact&>(new_contact),
                       EngineUtils::LifeCycle::OnTriggerEnter, A);
    new_contact.SetOther(A);
    LuaOnContactHandle(const_cast<const Contact&>(new_contact),
                       EngineUtils::LifeCycle::OnTriggerEnter, B);
  } else if (not fixtureA->IsSensor() and not fixtureB->IsSensor()) {
    LuaOnContactHandle(const_cast<const Contact&>(new_contact),
                       EngineUtils::LifeCycle::OnCollisionEnter, A);
    new_contact.SetOther(A);
    LuaOnContactHandle(const_cast<const Contact&>(new_contact),
                       EngineUtils::LifeCycle::OnCollisionEnter, B);
  }
}

void ContactListener::EndContact(b2Contact* contact) {
  Contact collision;
  auto A = Contact::GetActorA(contact);
  auto B = Contact::GetActorB(contact);

  if (not A or not B)
    return;

  auto fixtureA = contact->GetFixtureA();
  auto fixtureB = contact->GetFixtureB();

  collision.SetOther(B);
  collision.SetTriggerContact();
  collision.SetRelativeVelocity(fixtureA->GetBody()->GetLinearVelocity() -
                                fixtureB->GetBody()->GetLinearVelocity());

  if (fixtureA->IsSensor() and fixtureB->IsSensor()) {
    LuaOnContactHandle(const_cast<const Contact&>(collision),
                       EngineUtils::LifeCycle::OnTriggerExit, A);
    collision.SetOther(A);
    LuaOnContactHandle(const_cast<const Contact&>(collision),
                       EngineUtils::LifeCycle::OnTriggerExit, B);
  } else if (not fixtureA->IsSensor() and not fixtureB->IsSensor()) {
    LuaOnContactHandle(const_cast<const Contact&>(collision),
                       EngineUtils::LifeCycle::OnCollisionExit, A);
    collision.SetOther(A);
    LuaOnContactHandle(const_cast<const Contact&>(collision),
                       EngineUtils::LifeCycle::OnCollisionExit, B);
  }
}

void ContactListener::LuaOnContactHandle(const Contact& contact,
                                         EngineUtils::LifeCycle event,
                                         Actor* caller) {
  if (caller->lifecycle_function_map.count(event) <= 0) return;

  auto contact_functions = caller->lifecycle_function_map.at(event);
  for (const auto& [component_key, lua_func] : contact_functions) {
    try {
      const auto& component = caller->entity_components.at(component_key);
      lua_func(component, contact);
    } catch (luabridge::LuaException const& e) {
      Renderer::log_error(caller->name, e);
    }
  }
}