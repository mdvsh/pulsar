//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#include "Rigidbody.h"
#include <cmath>

const uint16 Rigidbody::CATEGORY_COLLIDER = 0x0001;  // Binary: 0000000000000001
const uint16 Rigidbody::CATEGORY_TRIGGER = 0x0002;   // Binary: 0000000000000010
const uint16 Rigidbody::CATEGORY_PHANTOM = 0x0004;   // Binary: 0000000000000100

Rigidbody::Rigidbody()
    : x(0.0f),
      y(0.0f),
      body_type("dynamic"),
      precise(true),
      gravity_scale(1.0f),
      density(1.0f),
      angular_friction(0.3f),
      rotation(0.0f),
      has_collider(true),
      has_trigger(true),
      type("Rigidbody"),
      key("???"),
      actor(nullptr),
      enabled(true),
      body(nullptr) {}

Rigidbody::~Rigidbody() {
  if (body) {
    body->GetWorld()->DestroyBody(body);
  }
}

void Rigidbody::Initialize() {
  b2BodyDef bodyDef;
  if (body_type == "static")
    bodyDef.type = b2_staticBody;
  else if (body_type == "kinematic")
    bodyDef.type = b2_kinematicBody;
  else
    bodyDef.type = b2_dynamicBody;

  bodyDef.position.Set(x, y);
  bodyDef.bullet = precise;
  bodyDef.angularDamping = angular_friction;
  bodyDef.gravityScale = gravity_scale;
  bodyDef.angle = to_radian(rotation);
  // rotation is in degrees, angle in radians

  if (not body)
    body = SceneManager::getInstance().GetPhysWorld()->CreateBody(&bodyDef);

  b2PolygonShape shape;
  shape.SetAsBox(0.5f, 0.5f);

  if (not has_collider and not has_trigger)
    CreatePhantomFixture();
  if (has_collider)
    CreateCollider();
  if (has_trigger)
    CreateTrigger();
}

void Rigidbody::CreatePhantomFixture() {
  auto* polygon_shape = new b2PolygonShape();
  polygon_shape->SetAsBox(collider_width * 0.5f, collider_height * 0.5f);
  b2FixtureDef fixture_def;

  fixture_def.shape = polygon_shape;
  fixture_def.density = density;
  fixture_def.isSensor = true;
  fixture_def.userData.pointer = reinterpret_cast<uintptr_t>(actor);
  fixture_def.filter.categoryBits = CATEGORY_PHANTOM;
  fixture_def.filter.maskBits = 0x0000;

  body->CreateFixture(&fixture_def);
}

void Rigidbody::CreateCollider() {
  b2Shape* shape = nullptr;
  if (collider_type == "box") {
    auto* polygon_shape = new b2PolygonShape();
    polygon_shape->SetAsBox(collider_width * 0.5f, collider_height * 0.5f);
    shape = polygon_shape;
  } else if (collider_type == "circle") {
    auto* circle_shape = new b2CircleShape();
    circle_shape->m_radius = collider_radius;
    shape = circle_shape;
  }
  b2FixtureDef fixture_def;
  fixture_def.shape = shape;
  fixture_def.density = density;
  fixture_def.isSensor = false;
  fixture_def.restitution = bounciness;
  fixture_def.friction = friction;
  fixture_def.userData.pointer = reinterpret_cast<uintptr_t>(actor);
  fixture_def.filter.categoryBits = CATEGORY_COLLIDER;
  fixture_def.filter.maskBits = CATEGORY_COLLIDER;
  body->CreateFixture(&fixture_def);
}

void Rigidbody::CreateTrigger() {
  b2Shape* shape = nullptr;
  if (trigger_type == "box") {
    auto* polygon_shape = new b2PolygonShape();
    polygon_shape->SetAsBox(trigger_width * 0.5f, trigger_height * 0.5f);
    shape = polygon_shape;
  } else if (trigger_type == "circle") {
    auto* circle_shape = new b2CircleShape();
    circle_shape->m_radius = trigger_radius;
    shape = circle_shape;
  }
  b2FixtureDef fixture_def;
  fixture_def.shape = shape;
  fixture_def.density = density;
  fixture_def.isSensor = true;
  fixture_def.restitution = bounciness;
  fixture_def.friction = friction;
  fixture_def.userData.pointer = reinterpret_cast<uintptr_t>(actor);
  fixture_def.filter.categoryBits = CATEGORY_TRIGGER;
  fixture_def.filter.maskBits = CATEGORY_TRIGGER;
  body->CreateFixture(&fixture_def);
}

void Rigidbody::SetPosition(const b2Vec2& pos) {
  if (not body) {
    x = pos.x;
    y = pos.y;
  } else {
    body->SetTransform(pos, body->GetAngle());
  }
}

void Rigidbody::SetRotation(float new_rotation) {
  body->SetTransform(body->GetPosition(), to_radian(new_rotation));
}

b2Vec2 Rigidbody::GetPosition() const {
  if (not body)
    return {x, y};
  return body->GetPosition();
}

float Rigidbody::GetRotation() const {
  return to_degree(body->GetAngle());
}

void Rigidbody::AddForce(const b2Vec2& force) {
  body->ApplyForceToCenter(force, true);
}

void Rigidbody::SetVelocity(const b2Vec2& velocity) {
  body->SetLinearVelocity(velocity);
}

void Rigidbody::SetAngularVelocity(float degrees_clockwise) {
  body->SetAngularVelocity(to_radian(degrees_clockwise));
}

void Rigidbody::SetGravityScale(float scale) {
  body->SetGravityScale(scale);
}

void Rigidbody::SetUpDirection(const b2Vec2& direction) {
  b2Vec2 normalised_direction = direction;
  normalised_direction.Normalize();
  body->SetTransform(body->GetPosition(), glm::atan(normalised_direction.x,
                                                    -normalised_direction.y));
}

void Rigidbody::SetRightDirection(const b2Vec2& direction) {
  b2Vec2 normalised_direction = direction;
  normalised_direction.Normalize();
  body->SetTransform(body->GetPosition(), glm::atan(normalised_direction.x,
                                                    -normalised_direction.y) -
                                              b2_pi / 2.0f);
}

b2Vec2 Rigidbody::GetVelocity() const {
  return body->GetLinearVelocity();
}

float Rigidbody::GetAngularVelocity() const {
  return to_degree(body->GetAngularVelocity());
}

float Rigidbody::GetGravityScale() const {
  return body->GetGravityScale();
}

b2Vec2 Rigidbody::GetUpDirection() const {
  float angle = body->GetAngle();
  b2Vec2 upDirection(glm::sin(angle), -glm::cos(angle));
  upDirection.Normalize();
  return upDirection;
}

b2Vec2 Rigidbody::GetRightDirection() const {
  float angle = body->GetAngle();
  b2Vec2 rightDirection(glm::cos(angle), glm::sin(angle));
  rightDirection.Normalize();
  return rightDirection;
}

float Rigidbody::to_degree(float radian) {
  return radian * (180.0f / b2_pi);
}

float Rigidbody::to_radian(float degree) {
  return degree * (b2_pi / 180.0f);
}
void Rigidbody::Destroy() {
  if (body) {
    body->GetWorld()->DestroyBody(body);
  }
}
