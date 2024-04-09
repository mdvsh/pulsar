//
// Created by Madhav Shekhar Sharma on 3/20/24.
//

#include "EventBus.h"
#include <algorithm>

namespace App {
std::unordered_map<std::string, std::vector<event_pair>>
    EventBus::subscription_map;
std::vector<std::tuple<std::string, luabridge::LuaRef, luabridge::LuaRef>>
    EventBus::pending_subscriptions;
std::vector<std::tuple<std::string, luabridge::LuaRef, luabridge::LuaRef>>
    EventBus::pending_unsubscriptions;

void EventBus::Publish(const std::string& event_type,
                       const luabridge::LuaRef& event_obj) {
  auto it = subscription_map.find(event_type);
  if (it != subscription_map.end()) {
    const auto& subscribers = it->second;
    for (const auto& [subscriber, callback] : subscribers) {
      callback(subscriber, event_obj);
    }
  }
}

void EventBus::Subscribe(const std::string& event_type,
                         const luabridge::LuaRef& subscriber,
                         const luabridge::LuaRef& callback) {
  pending_subscriptions.emplace_back(event_type, subscriber, callback);
}

void EventBus::Unsubscribe(const std::string& event_type,
                           const luabridge::LuaRef& subscriber,
                           const luabridge::LuaRef& callback) {
  pending_unsubscriptions.emplace_back(event_type, subscriber, callback);
}

void EventBus::ProcessPendingSubscriptions() {
  for (const auto& [event_type, subscriber, callback] : pending_subscriptions) {
    subscription_map[event_type].emplace_back(subscriber, callback);
  }
  pending_subscriptions.clear();
}

void EventBus::ProcessPendingUnsubscriptions() {
  for (const auto& unsubscription : pending_unsubscriptions) {
    const auto& event_type = std::get<0>(unsubscription);
    const auto& subscriber = std::get<1>(unsubscription);
    const auto& callback = std::get<2>(unsubscription);
    auto it = subscription_map.find(event_type);
    if (it != subscription_map.end()) {
      auto& subscribers = it->second;
      auto pred = [&](const event_pair& p) {
        return p.first == subscriber && p.second == callback;
      };
      subscribers.erase(
          std::remove_if(subscribers.begin(), subscribers.end(), pred),
          subscribers.end());
    }
  }
  pending_unsubscriptions.clear();
}
}  // namespace App