#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace config
{
  static const int NUM_SPRITES_X = 2;
  static const int NUM_SPRITES_Y = 4;

  static const int MAX_LAYERS = 2;
  static const int TILE_W = 16;
  static const int TILE_H = 16;
  static const int MAX_ZONES = 16;

  static const int FPS = 60;

  static const int GAME_RES_X = 256;
  static const int GAME_RES_Y = 240;

  void load_config();

  std::string get(const std::string& key);
  void set(const std::string& key, const std::string& value);
}

#endif
