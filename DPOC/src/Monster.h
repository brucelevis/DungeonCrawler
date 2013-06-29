#ifndef MONSTER_H
#define MONSTER_H

#include <string>
#include <map>

#include <SFML/Graphics.hpp>

static const std::string BUG_MONSTER = "BUG_MONSTER";

struct MonsterDef
{
  std::string name;
  std::string description;

  std::map<std::string, int> attributeMap;

  std::string texture;
  sf::IntRect textureRect;
};

MonsterDef get_monster_definition(const std::string& name);
std::string get_monster_description(const std::string& name);

#endif
