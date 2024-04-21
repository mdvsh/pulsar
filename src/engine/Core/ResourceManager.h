//
// Created by Madhav Shekhar Sharma on 4/20/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_RESOURCEMANAGER_H_
#define PULSAR_SRC_ENGINE_CORE_RESOURCEMANAGER_H_

#include <atomic>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <yyjson.h>

#include "Core/Resources.hpp"

namespace App {

class ResourceManager {
 public:
  struct SceneData {
    std::string name;
    std::string path;
    std::vector<std::string> actors;
  };

  struct MediaData {
    std::string name;
    std::string path;
    // later, thumbnail, type, etc.
  };

  static ResourceManager& getInstance() {
    static ResourceManager instance;
    return instance;
  }

  // Delete copy constructor and assignment operator to prevent copying
  ResourceManager(const ResourceManager&) = delete;
  ResourceManager& operator=(const ResourceManager&) = delete;

  [[nodiscard]] std::vector<std::string> getScenes() const {
    return m_scene_names;
  }

  [[nodiscard]] std::vector<std::string> getMedia() const {
    return m_media_names;
  }

  [[nodiscard]] std::shared_ptr<SceneData> getScene(const std::string& name) {
    return m_scenes[name];
  }

  [[nodiscard]] std::shared_ptr<MediaData> getMedia(const std::string& name) {
    return m_media[name];
  }

  void wb_edited_file(const std::string& file_path, const std::string& content);
  [[nodiscard]] std::string r_editor_file(const std::string& file_path);

  void evict_from_resources_cache(const std::string& file_path);
  void read_scene(const std::string& name, const std::string& path);
  void update_scene_data(yyjson_doc *doc, std::shared_ptr<SceneData> scene);

 private:
  std::unordered_map<std::string, std::shared_ptr<SceneData>> m_scenes;
  std::unordered_map<std::string, std::shared_ptr<MediaData>> m_media;
  std::unordered_map<std::string,
                     std::pair<std::string, std::filesystem::file_time_type>> editor_file_cache;
  std::vector<std::string> m_scene_names;
  std::vector<std::string> m_media_names;

  std::thread observer_thread;
  std::atomic<bool> halt_observer{false};
  std::mutex cache_mutex;

  const std::filesystem::path resources_path = Resources::game_path();
  const std::unordered_set<std::string> supported_extensions = {
      ".png", ".jpg", ".jpeg", ".bmp", ".wav", ".mp3"};

  ResourceManager();
  ~ResourceManager();
  void observeResources();
  void scan_for_changes();

};

}  // namespace App

#endif  // PULSAR_SRC_ENGINE_CORE_RESOURCEMANAGER_H_
