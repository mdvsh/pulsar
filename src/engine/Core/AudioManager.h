//
// Created by Madhav Shekhar Sharma on 4/8/24.
//

#ifndef PULSAR_SRC_ENGINE_CORE_AUDIOMANAGER_H_
#define PULSAR_SRC_ENGINE_CORE_AUDIOMANAGER_H_

#include <SDL2_mixer/SDL_mixer.h>
#include <rapidjson/document.h>
#include <unordered_map>
#include <string>
#include <vector>

class AudioManager {
  static std::unordered_map<std::string, Mix_Chunk*> audio_clips;
  AudioManager() {}
  const std::string AUDIO_PATH = "resources/audio/";
  const std::string INTRO = "intro_bgm";
  const std::vector<std::string> EXTS = {".wav", ".ogg"};

 public:
  AudioManager(const AudioManager&) = delete;
  AudioManager& operator=(const AudioManager&) = delete;

  static AudioManager& getInstance() {
    static AudioManager instance;
    return instance;
  }

  bool loaded_intro_music = false;
  bool halted_intro_music = false;
  static void initialize();
  static void start_intro_music(const rapidjson::Document& game_config);
  static void stop_intro_music();

  [[maybe_unused]] static void load_audio_clips(const rapidjson::Document& config,
                                                const std::string& audio_key,
                                                const std::string& store_as);
  static void play_channel(const std::string& key, int channel, int loops);
  static void LuaPlayChannel(int channel,
                             const std::string& effectName,
                             bool doesLoop);
  static void halt_channel(int channel);
  static void load_sfx(const std::string& key);
  static void SetVolume(int channel, float volume);
  [[maybe_unused]] const int SCORE_CHANNEL = 1;
};

#endif  // PULSAR_SRC_ENGINE_CORE_AUDIOMANAGER_H_
