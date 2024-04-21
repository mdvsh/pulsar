//
// Created by Madhav Shekhar Sharma on 4/20/24.
//

#include "ResourceManager.h"
#include <fstream>
#include <future>
#include <sstream>

namespace App {

ResourceManager::ResourceManager() {
  scan_for_changes();
  observer_thread = std::thread(&ResourceManager::observeResources, this);
}

void ResourceManager::observeResources() {
  while (not halt_observer) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    scan_for_changes();
  }
}

void ResourceManager::evict_from_resources_cache(const std::string& file_path) {
  std::string name = std::filesystem::path(file_path).stem().string();
  if (m_scenes.find(name) != m_scenes.end()) {
    m_scenes.erase(name);
    m_scene_names.erase(
        std::remove(m_scene_names.begin(), m_scene_names.end(), name),
        m_scene_names.end());
  }
  if (m_media.find(name) != m_media.end()) {
    m_media.erase(name);
    m_media_names.erase(
        std::remove(m_media_names.begin(), m_media_names.end(), name),
        m_media_names.end());
  }
}

void ResourceManager::read_scene(const std::string& name,
                                 const std::string& path) {
  std::ifstream file(path);
  if (file.good()) {
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    yyjson_doc* doc = yyjson_read(content.c_str(), content.length(), 0);
    if (doc) {
      auto it = m_scenes.find(name);
      if (it != m_scenes.end()) {
        auto scene = it->second;
        update_scene_data(doc, scene);
      } else {
        auto scene = std::make_shared<SceneData>();
        scene->name = name;
        scene->path = path;
        update_scene_data(doc, scene);
        m_scenes[name] = scene;
        m_scene_names.push_back(name);
      }
      yyjson_doc_free(doc);
    }
  }
}

void ResourceManager::update_scene_data(yyjson_doc* doc,
                                        std::shared_ptr<SceneData> scene) {
  scene->actors.clear();
  yyjson_val* actors_val = yyjson_obj_get(doc->root, "actors");
  size_t idx, max;
  if (actors_val && yyjson_is_arr(actors_val)) {
    size_t actors_count = yyjson_arr_size(actors_val);
    scene->actors.reserve(actors_count);
    yyjson_val* actor_val;
    yyjson_arr_foreach(actors_val, idx, max, actor_val) {
      if (yyjson_is_obj(actor_val)) {
        yyjson_val* name_val = yyjson_obj_get(actor_val, "name");
        if (name_val && yyjson_is_str(name_val)) {
          std::string actor_name = yyjson_get_str(name_val);
          scene->actors.push_back(actor_name);
        }
      }
    }
  }
}

void ResourceManager::scan_for_changes() {
  for (const auto& entry :
       std::filesystem::directory_iterator(resources_path / "scenes/")) {
    if (entry.is_regular_file() && entry.path().extension() == ".scene") {
      std::string sceneName = entry.path().stem().string();
      read_scene(sceneName, entry.path().generic_string());
      // if (m_scenes.find(sceneName) == m_scenes.end()) {
      //   auto scene = std::make_shared<SceneData>();
      //   scene->name = sceneName;
      //   scene->path = entry.path().generic_string();
      //   m_scenes[sceneName] = scene;
      //   m_scene_names.push_back(sceneName);
      // }
    }
  }

  for (const auto& entry :
       std::filesystem::recursive_directory_iterator(resources_path)) {
    if (entry.is_regular_file() &&
        supported_extensions.find(entry.path().extension()) !=
            supported_extensions.end()) {
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

ResourceManager::~ResourceManager() {
  halt_observer = true;
  if (observer_thread.joinable()) {
    observer_thread.join();
  }
}

std::string ResourceManager::r_editor_file(const std::string& file_path) {
  std::lock_guard<std::mutex> lock(cache_mutex);
  auto file_time = std::filesystem::last_write_time(file_path);
  if (editor_file_cache.find(file_path) != editor_file_cache.end() &&
      editor_file_cache[file_path].second == file_time) {
    // File content is cached and timestamp matches, return from cache
    return editor_file_cache[file_path].first;
  }
  std::ifstream file(file_path);
  if (file.good()) {
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    editor_file_cache[file_path] = std::make_pair(content, file_time);
    file.close();
    return content;
  }
  return "";
}

void ResourceManager::wb_edited_file(const std::string& file_path,
                                     const std::string& content) {
  [[maybe_unused]] std::future<void> future =
      std::async(std::launch::async, [this, file_path, content] {
        std::lock_guard<std::mutex> lock(cache_mutex);
        auto file_time = std::filesystem::last_write_time(file_path);
        if (editor_file_cache.find(file_path) != editor_file_cache.end() &&
            editor_file_cache[file_path].first == content &&
            editor_file_cache[file_path].second == file_time) {
          // File content and timestamp are the same, no need to write
          return;
        }
        std::ofstream file(file_path);
        if (file.is_open()) {
          file << content;
          file.close();
          editor_file_cache[file_path] = std::make_pair(content, file_time);
          // evict_from_resources_cache(file_path);
        }
      });
}

}  // namespace App
