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
  static const int FONT_SIZE = 8;

  static const int GAME_RES_X = 320;
  static const int GAME_RES_Y = 240;

  static const int RAYCASTER_RES_X = 320;
  static const int RAYCASTER_RES_Y = 160;

  static const int MESSAGE_BOX_WIDTH = 256;
  static const int MESSAGE_BOX_HEIGHT = 64;
  static const int MESSAGE_BOX_BOTTOM_MARGIN = 70;

  static const int THING_MENU_WIDTH = GAME_RES_X - 32;
  static const int THING_MENU_HEIGHT = GAME_RES_Y - 48;

  static const int EQUIP_MENU_STATS_WIDTH = 128;
  static const int EQUIP_MENU_PARTS_WIDTH = GAME_RES_X - EQUIP_MENU_STATS_WIDTH;
  static const int EQUIP_MENU_PARTS_HEIGHT = 112;
  static const int EQUIP_MENU_MAX_EQUIPPED_ITEM_NAME_LENGTH = 16;

  static const int STATUS_SCREEN_WIDTH = 240;
  static const int STATUS_SCREEN_HEIGHT = 208;

  static std::string RESOURCE_DIR = "Resources/";

  void load_config();

  std::string get(const std::string& key);
  void set(const std::string& key, const std::string& value);
  bool isSet(const std::string& key);

  inline std::string res_path(const std::string& path)
  {
    return RESOURCE_DIR + path;
  }

  extern bool ENCOUNTERS_ENABLED;
}

#endif
