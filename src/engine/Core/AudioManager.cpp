//
// Created by Madhav Shekhar Sharma on 2/10/24.
//

#include "AudioManager.h"

#include <filesystem>
#include <string>

#include "AudioHelper.h"

std::unordered_map<std::string, Mix_Chunk*> AudioManager::audio_clips;

void AudioManager::initialize() {
  if (AudioHelper::Mix_OpenAudio498(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    std::cout << "[my] error: failed to initialize audio";
  }
  AudioHelper::Mix_AllocateChannels498(50);
}

void AudioManager::start_intro_music(const rapidjson::Document& game_config) {
  std::string base_path = AudioManager::getInstance().AUDIO_PATH;
  if (const auto key = AudioManager::getInstance().INTRO.c_str();
      game_config.HasMember(key) && game_config[key].IsString()) {
    const std::string intro_bgm = game_config[key].GetString();
    base_path += intro_bgm;
    for (const auto& ext : AudioManager::getInstance().EXTS) {
      std::string path = base_path;
      path += ext;
      if (std::filesystem::exists(path)) {
        const auto intro_bgm_mix = AudioHelper::Mix_LoadWAV498(path.c_str());
        AudioHelper::Mix_PlayChannel498(0, intro_bgm_mix, -1);
        AudioManager::getInstance().loaded_intro_music = true;
        return;
      }
    }
    std::cout << "error: failed to play audio clip " << intro_bgm;
    std::exit(0);
  }
}
void AudioManager::stop_intro_music() {
  if (AudioManager::getInstance().loaded_intro_music and
      not AudioManager::getInstance().halted_intro_music) {
    AudioHelper::Mix_HaltChannel498(0);
    AudioManager::getInstance().halted_intro_music = true;
  }
}

[[maybe_unused]] void AudioManager::load_audio_clips(const rapidjson::Document& config,
                                                     const std::string& audio_key,
                                                     const std::string& store_as) {
  if (audio_clips.find(audio_key) != audio_clips.end())
    return;
  if (const auto key = audio_key.c_str();
      config.HasMember(key) and config[key].IsString()) {
    const std::string audio_clip = config[key].GetString();
    std::string base_path = AudioManager::getInstance().AUDIO_PATH;
    base_path += audio_clip;
    for (const auto& ext : AudioManager::getInstance().EXTS) {
      std::string path = base_path;
      path += ext;
      if (std::filesystem::exists(path)) {
        const auto audio_clip_mix = AudioHelper::Mix_LoadWAV498(path.c_str());
        if (store_as.empty()) {
          audio_clips[audio_key] = audio_clip_mix;
        } else {
          audio_clips[store_as] = audio_clip_mix;
        }
        return;
      }
    }
  }
}
void AudioManager::play_channel(const std::string& key,
                                const int channel,
                                const int loops) {
  if (key.empty())
    return;
  if (audio_clips.find(key) == audio_clips.end()) {
    //        std::cout << key << " audio requested on " << channel <<
    //        std::endl;
    load_sfx(key);
  }
  AudioHelper::Mix_PlayChannel498(channel, audio_clips[key], loops);
}

void AudioManager::LuaPlayChannel(int channel,
                                  const std::string& effectName,
                                  bool doesLoop) {
  const int loop = doesLoop ? -1 : 0;
  return play_channel(effectName, channel, loop);
}

void AudioManager::halt_channel(const int channel) {
  AudioHelper::Mix_HaltChannel498(channel);
}

void AudioManager::load_sfx(const std::string& key) {
  if (key.empty())
    return;
  if (audio_clips.find(key) != audio_clips.end())
    return;
  std::string base_path = AudioManager::getInstance().AUDIO_PATH;
  base_path += key;
  for (const auto& ext : AudioManager::getInstance().EXTS) {
    std::string path = base_path;
    path += ext;
    if (std::filesystem::exists(path)) {
      const auto audio_clip_mix = AudioHelper::Mix_LoadWAV498(path.c_str());
      audio_clips[key] = audio_clip_mix;
      return;
    }
  }
}

void AudioManager::SetVolume(int channel, float volume) {
  volume = std::max(0, std::min(static_cast<int>(volume), 128));
  AudioHelper::Mix_Volume498(channel, volume);
}
