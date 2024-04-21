//
// Created by Madhav Shekhar Sharma on 4/20/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_RESOURCEMANAGER_H_
#define PULSAR_SRC_ENGINE_CORE_RESOURCEMANAGER_H_

#include <filesystem>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <atomic>

#include "Core/Resources.hpp"


namespace App {

class ResourceManager {
 public:
  struct SceneData {
    std::string name;
    std::string path;
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

 private:
  std::unordered_map<std::string, std::shared_ptr<SceneData>> m_scenes;
  std::unordered_map<std::string, std::shared_ptr<MediaData>> m_media;
  std::vector<std::string> m_scene_names;
  std::vector<std::string> m_media_names;

  std::thread observer_thread;
  std::atomic<bool> halt_observer{false};

  const std::filesystem::path resources_path = Resources::game_path();
  const std::unordered_set<std::string> supported_extensions = {".png", ".jpg", ".jpeg",
                                                   ".bmp", ".wav", ".mp3"};

  ResourceManager() {
    scan_for_changes();
    observer_thread = std::thread(&ResourceManager::observeResources, this);
  }

  ~ResourceManager() {
    halt_observer = true;
    if (observer_thread.joinable()) {
      observer_thread.join();
    }
  }

  void observeResources() {
    while (not halt_observer) {
      std::this_thread::sleep_for(std::chrono::seconds(3));
      scan_for_changes();
    }
  }

  void scan_for_changes() {
    for (const auto& entry :
         std::filesystem::directory_iterator(resources_path / "scenes/")) {
      if (entry.is_regular_file() && entry.path().extension() == ".scene") {
        std::string sceneName = entry.path().stem().string();
        if (m_scenes.find(sceneName) == m_scenes.end()) {
          auto scene = std::make_shared<SceneData>();
          scene->name = sceneName;
          scene->path = entry.path().generic_string();
          m_scenes[sceneName] = scene;
          m_scene_names.push_back(sceneName);
        }
      }
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(resources_path)) {
      if (entry.is_regular_file() && supported_extensions.find(entry.path().extension()) != supported_extensions.end()) {
        std::string mediaName = entry.path().stem().string();
        if (m_media.find(mediaName) == m_media.end()) {
          auto media = std::make_shared<MediaData>();
          media->name = mediaName;
          media->path = entry.path().generic_string();
          m_media[mediaName] = media;
          m_media_names.push_back(mediaName);
        }
      }
    }

  }
};

}  // namespace App

#endif  // PULSAR_SRC_ENGINE_CORE_RESOURCEMANAGER_H_
