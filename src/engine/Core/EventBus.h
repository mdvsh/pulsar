//
// Created by Madhav Shekhar Sharma on 3/20/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_EVENTBUS_H_
#define PULSAR_SRC_ENGINE_CORE_EVENTBUS_H_


#include <unordered_map>
#include <utility>

// clang-format off
#include "lua.hpp"
#include <LuaBridge/LuaBridge.h>
// clang-format on

typedef std::pair<luabridge::LuaRef, luabridge::LuaRef> event_pair;

namespace App {

class EventBus {
 public:
  static std::unordered_map<std::string, std::vector<event_pair>>
      subscription_map;
  static std::vector<
      std::tuple<std::string, luabridge::LuaRef, luabridge::LuaRef>>
      pending_subscriptions;
  static std::vector<
      std::tuple<std::string, luabridge::LuaRef, luabridge::LuaRef>>
      pending_unsubscriptions;

  static void Publish(const std::string& event_type,
                      const luabridge::LuaRef& event_obj);
  static void Subscribe(const std::string& event_type,
                        const luabridge::LuaRef& subscriber,
                        const luabridge::LuaRef& callback);
  static void Unsubscribe(const std::string& event_type,
                          const luabridge::LuaRef& subscriber,
                          const luabridge::LuaRef& callback);
  static void ProcessPendingSubscriptions();
  static void ProcessPendingUnsubscriptions();
};

}  // namespace App

#endif  // PULSAR_SRC_ENGINE_CORE_EVENTBUS_H_
