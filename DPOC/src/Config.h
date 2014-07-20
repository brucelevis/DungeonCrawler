#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace config
{
  static const int NUM_SPRITES_X = 2;
  static const int NUM_SPRITES_Y = 4;

  static const int MAX_LAYERS = 2;
  extern int TILE_W;
  extern int TILE_H;
  static const int MAX_ZONES = 16;

  static const int FPS = 60;

  static const int GAME_RES_X = 640;
  static const int GAME_RES_Y = 480;

  static const int RAYCASTER_RES_X = 320;
  static const int RAYCASTER_RES_Y = 200;

  static std::string RESOURCE_DIR = "Resources_2/";

  void load_config();

  std::string get(const std::string& key);
  void set(const std::string& key, const std::string& value);

  inline std::string res_path(const std::string& path)
  {
    return RESOURCE_DIR + path;
  }
}

#endif
