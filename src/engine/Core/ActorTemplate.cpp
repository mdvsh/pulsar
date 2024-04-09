//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#include "ActorTemplate.h"
#include "ECS.h"
#include "EngineUtils.h"
#include "SceneManager.h"

#include <rapidjson/document.h>
#include <filesystem>

Actor ActorTemplate::load_actor_template(const std::string& template_name) {
  const std::string templates_path = "resources/actor_templates";
  const std::string actor_templates_path =
      templates_path + "/" + template_name + ".template";

  if (not std::filesystem::exists(actor_templates_path)) {
    std::cout << "error: template " << template_name << " is missing";
    std::exit(0);
  }
  rapidjson::Document actor_templates;
  EngineUtils::ReadJsonFile(actor_templates_path, actor_templates);

  Actor actor;
  update_from_source_file(actor, actor_templates);
  actor.set_id();
  return actor;
}
void ActorTemplate::update_from_source_file(Actor& actor,
                                            const rapidjson::Value& actorData) {
  if (actorData.HasMember("name"))
    actor.name = actorData["name"].GetString();

  if (actorData.HasMember(("components")) and
      actorData["components"].IsObject()) {
    auto& ecs = App::ECS::getInstance();
    const auto& raw_components = actorData["components"].GetObject();
    for (auto& component : raw_components) {
      const std::string component_key = component.name.GetString();
      auto inserted_at = actor.entity_components.end();
      //	  auto comp_type = ECS::ComponentType::LUA;

      if (const auto component_itr =
              actor.entity_components.find(component_key);
          component_itr == actor.entity_components.end()) {
        if (component.value.HasMember("type") and
            component.value["type"].IsString()) {
          const std::string component_name =
              component.value["type"].GetString();
          auto component_pair =
              ecs.create_component(component_key, component_name);
          inserted_at = actor.entity_components
                            .emplace(component_key, component_pair.second)
                            .first;
          //		  comp_type = component_pair.first;
        }
      } else {
        luabridge::LuaRef template_inherited_component_table =
            luabridge::newTable(ecs.get_lua_state());
        ecs.establish_inheritance(template_inherited_component_table,
                                  component_itr->second);
        component_itr->second = template_inherited_component_table;
        template_inherited_component_table["key"] = component_key;
        inserted_at = component_itr;
      }

      if (inserted_at != actor.entity_components.end()) {
        auto& component_table = inserted_at->second;
        auto component_properties = component.value.GetObject();

        //		if (comp_type == ECS::ComponentType::LUA) {
        //		  if (component_table["enabled"].isNil())
        // component_table["enabled"] = true;
        //                    if (component_table["OnStartOver"].isNil())
        //                    component_table["OnStartOver"] = false;
        // } // ::CPP components already have these exposed.

        actor.populate_lifecycle_functions(component_table, component_key);

        for (auto& value : component_properties) {
          const std::string property = value.name.GetString();
          if (property == "type") {
            actor.entity_components_by_type[value.value.GetString()].insert(
                component_key);
            continue;
          }
          if (value.value.IsString()) {
            component_table[property] = value.value.GetString();
            actor.entity_component_properties.push_back(property);
          } else if (value.value.IsInt()) {
            component_table[property] = value.value.GetInt();
            actor.entity_component_properties.push_back(property);
          } else if (value.value.IsFloat()) {
            component_table[property] = value.value.GetFloat();
            actor.entity_component_properties.push_back(property);
          } else if (value.value.IsBool()) {
            component_table[property] = value.value.GetBool();
            actor.entity_component_properties.push_back(property);
          }
        }
      }
    }
  }
}
