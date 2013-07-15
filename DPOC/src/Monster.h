#ifndef MONSTER_H
#define MONSTER_H

#include <string>
#include <map>

#include <SFML/Graphics.hpp>

static const std::string BUG_MONSTER = "BUG_MONSTER";

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

  std::vector<MonsterActionEntry> actions;
  std::vector<MonsterDropItem> itemDrop;

  std::map<std::string, float> resistance;

  std::vector<std::string> immunity;
};

void load_monsters();

MonsterDef get_monster_definition(const std::string& name);
std::string get_monster_description(const std::string& name);
std::vector<std::string> monster_drop_items(const MonsterDef& monster);

#endif
