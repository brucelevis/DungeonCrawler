#ifndef MONSTER_H
#define MONSTER_H

#include <string>
#include <map>

#include <SFML/Graphics.hpp>

#include "Effect.h"

struct MonsterActionEntry
{
  std::string action;
  std::string objectName;
  int weight;
};

struct MonsterDropItem
{
  std::string itemName;
  int chance;
};

struct MonsterDef
{
  std::string name;
  std::string description;

  std::map<std::string, int> attributeMap;

  std::string texture;
  sf::IntRect textureRect;
  sf::Color color;
  float scale;

  std::vector<MonsterActionEntry> actions;
  std::vector<MonsterDropItem> itemDrop;
  std::vector<std::string> stealItems;

  std::map<std::string, float> resistance;

  std::vector<std::string> immunity;

  int numberOfAttacks;

  Effect attackEffect;
};

void load_monsters();

MonsterDef get_monster_definition(const std::string& name);
std::string get_monster_description(const std::string& name);
std::vector<std::string> monster_drop_items(const MonsterDef& monster);

#endif
