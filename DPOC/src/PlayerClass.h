#ifndef PLAYER_CLASS_H
#define PLAYER_CLASS_H

#include <map>
#include <vector>
#include <string>

#include <SFML/Graphics.hpp>

#include "coord.h"

struct BaseAttr
{
  int base, max;
};

struct PlayerClass
{
  std::string name;
  std::map<std::string, BaseAttr> baseAttributes;
  std::map<int, std::vector<std::string> > spells;
  std::vector<std::string> equipment;

  std::vector<std::string> battleActions;

  std::string texture;
  sf::IntRect textureBlock;

  std::string faceTexture;
  sf::IntRect textureRect;
};

void load_classes();

PlayerClass player_class_ref(const std::string& className);

#endif
