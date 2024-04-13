//
// Created by Madhav Shekhar Sharma on 1/30/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_ENGINEUTILS_H_
#define PULSAR_SRC_ENGINE_CORE_ENGINEUTILS_H_


#include <iostream>
#include <string>

#include <unordered_map>
#include <utility>
#include "glm/glm.hpp"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

#include <box2d/box2d.h>

class EngineUtils {
 public:
  enum class MoveIntent {
    none,
    north,
    south,
    east,
    west,
  };
  enum class LifeCycle { OnStart, OnUpdate, OnLateUpdate, OnDestroy, OnCollisionEnter, OnCollisionExit, OnTriggerEnter, OnTriggerExit };
  const std::unordered_map<LifeCycle, std::string> LifeCycleFunctionNameMap = {
      {LifeCycle::OnStart, "OnStart"},
      {LifeCycle::OnUpdate, "OnUpdate"},
      {LifeCycle::OnLateUpdate, "OnLateUpdate"},
      {LifeCycle::OnDestroy, "OnDestroy"},
      {LifeCycle::OnCollisionEnter, "OnCollisionEnter"},
      {LifeCycle::OnCollisionExit, "OnCollisionExit"},
      {LifeCycle::OnTriggerEnter, "OnTriggerEnter"},
      {LifeCycle::OnTriggerExit, "OnTriggerExit"}
  };

  // [SUS!?]
  static MoveIntent get_direction_intent(const float x, const float y) {
    auto mi = MoveIntent::none;
    if (x > 0)
      mi = MoveIntent::east;
    if (x < 0)
      mi = MoveIntent::west;
    if (y > 0)
      mi = MoveIntent::south;
    if (y < 0)
      mi = MoveIntent::north;
    return mi;
  }

  struct hash_pair {
    size_t operator()(const glm::ivec2& vec) const {
      return std::hash<int>()(vec.x) ^ (std::hash<int>()(vec.y) << 1);
    }
  };

  static void ReadJsonFile(const std::string& path,
                           rapidjson::Document& out_document) {
    FILE* file_pointer = nullptr;
#ifdef _WIN32
    fopen_s(&file_pointer, path.c_str(), "rb");
#else
    file_pointer = fopen(path.c_str(), "rb");
#endif
    char buffer[65536];
    rapidjson::FileReadStream stream(file_pointer, buffer, sizeof(buffer));
    out_document.ParseStream(stream);
    std::fclose(file_pointer);

    if (out_document.HasParseError()) {
      const rapidjson::ParseErrorCode errorCode = out_document.GetParseError();
      std::cout << "error parsing json at [" << path << "]" << std::endl;
      std::cout << "error code : " << errorCode << std::endl;
      exit(0);
    }
  }

  static std::string LoadStringFromJson(const rapidjson::Document& document,
                                        const std::string& key) {
    if (document.HasMember(key.c_str()) && document[key.c_str()].IsString()) {
      return document[key.c_str()].GetString();
    }
    return "";  // Return empty string if key is not found or not a string
  }

  static int LoadIntFromJson(const rapidjson::Document& document,
                             const std::string& key) {
    if (document.HasMember(key.c_str()) && document[key.c_str()].IsInt()) {
      return document[key.c_str()].GetInt();
    }
    return 0;  // Return 0 if key is not found or not an int
  }

  static int LoadIntFromJsonNegReturn(const rapidjson::Document& document,
                                      const std::string& key) {
    if (document.HasMember(key.c_str()) && document[key.c_str()].IsInt()) {
      return document[key.c_str()].GetInt();
    }
    return -1;  // Return -1 if key is not found or not an int
  }

  static float LoadFloatFromJson(const rapidjson::Document& document,
                                 const std::string& key) {
    if (document.HasMember(key.c_str())) {
      return document[key.c_str()].GetFloat();
    }
    return -1.0f;
  }

  static uint64_t create_composite_key(const int x, const int y) {
    auto x_ = static_cast<uint32_t>(x);
    auto y_ = static_cast<uint32_t>(y);
    return (static_cast<uint64_t>(x_) << 32) | static_cast<uint64_t>(y_);
  }

  //      TODO: Investigate later, somehow this doesn't erase the key properly
  //      (old) in update need new hash

  static bool is_phantom_fixture(const b2Fixture* fixture) {
    const auto& filter_data = fixture->GetFilterData();
    return (filter_data.categoryBits & filter_data.maskBits) == 0;
  }
};

enum class IMGType { Scene, UI, Pixel };
struct IMGRenderRequest {
  IMGType type;
  std::string image_name;
  float x, y;
  int render_order = 0;

  int r = 255, g = 255, b = 255, a = 255;
  float pivot_x = -1.0f, pivot_y = -1.0f;
  float rotation_degrees = 0.0f, scale_x = 1.0f, scale_y = 1.0f;

  IMGRenderRequest(IMGType type,
                   std::string image_name,
                   float x,
                   float y,
                   int render_order = 0)
      : type(type),
        image_name(std::move(image_name)),
        x(x),
        y(y),
        render_order(render_order) {}
};


#endif  // PULSAR_SRC_ENGINE_CORE_ENGINEUTILS_H_
