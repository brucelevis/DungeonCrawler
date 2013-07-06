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

  static const std::string SOUND_CANCEL = "Resources/Audio/Cancel1.wav";
  static const std::string SOUND_USE_ITEM = "Resources/Audio/Item2.wav";
  static const std::string SOUND_SUCCESS = "Resources/Audio/Sucess2.wav";
  static const std::string SOUND_EQUIP = "Resources/Audio/Equip1.ogg";
  static const std::string SOUND_ATTACK = "Resources/Audio/attack.wav";
  static const std::string SOUND_HIT = "Resources/Audio/hit.wav";
  static const std::string SOUND_ENEMY_HIT = "Resources/Audio/enemy-hit.wav";
  static const std::string SOUND_SPELL = "Resources/Audio/spell.wav";
  static const std::string SOUND_ESCAPE = "Resources/Audio/Run.ogg";
  static const std::string SOUND_MISS = "Resources/Audio/Evasion1.wav";
  static const std::string SOUND_HEAL = "Resources/Audio/blipp.wav";
  static const std::string SOUND_POISON = "Resources/Audio/Poison.wav";
}

#endif
