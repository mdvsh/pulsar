//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_ACTORTEMPLATE_H_
#define PULSAR_SRC_ENGINE_CORE_ACTORTEMPLATE_H_

#include <optional>
#include <unordered_map>

#include "Actor.h"

class ActorTemplate {
 public:
  static Actor load_actor_template(const std::string& template_name);
  static void update_from_source_file(Actor& actor,
                                      const rapidjson::Value& actorData);
};


#endif  // PULSAR_SRC_ENGINE_CORE_ACTORTEMPLATE_H_
