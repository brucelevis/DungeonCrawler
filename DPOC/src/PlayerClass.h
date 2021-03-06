#ifndef PLAYER_CLASS_H
#define PLAYER_CLASS_H

#include <map>
#include <vector>
#include <string>

#include <SFML/Graphics.hpp>

#include "Effect.h"
#include "coord.h"

struct BaseAttr
{
  int base, max;
};

struct PlayerClass
{
  std::string name;
  std::map<std::string, BaseAttr> baseAttributes;
  std::map<std::string, int> fixedAttributes; // Attributes increased with a fixed number each level.
  std::map<int, std::vector<std::string> > spells;
  std::vector<std::string> equipment;
  std::vector<std::string> startingEquipment;

  std::vector<std::string> battleActions;

  std::string texture;
  sf::IntRect textureBlock;

  std::string faceTexture;
  sf::IntRect textureRect;

  std::string description;
  Effect unarmedAttackEffect;
};

void load_classes();

PlayerClass player_class_ref(const std::string& className);
std::vector<PlayerClass> get_all_classes();

#endif
