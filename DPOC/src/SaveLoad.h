#ifndef SAVE_LOAD_H
#define SAVE_LOAD_H

#include <string>
#include <vector>
#include <map>

#include "Character.h"
#include "Direction.h"

struct EntityData
{
  std::string name;
  std::string tag;
  std::string spriteName;
  int x;
  int y;
  Direction dir;
  bool walkThrough;
  float speed;
};

struct CharacterData
{
  std::string name;
  std::map<std::string, std::string> equipment;
  std::vector<std::string> spells;
  std::string className;
  std::string textureName;
  int textureX, textureY, textureW, textureH;
  std::map<std::string, Attribute> attributes;
  std::vector<std::string> statusEffects;
};

void save_game(const std::string& saveFile);
void load_game(const std::string& saveFile);
CharacterData get_party_leader_from_save(const std::string& saveFile);

#endif
