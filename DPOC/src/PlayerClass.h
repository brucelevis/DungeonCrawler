#ifndef PLAYER_CLASS_H
#define PLAYER_CLASS_H

#include <map>
#include <vector>
#include <string>

struct PlayerClass
{
  std::string name;
  std::map<std::string, int> baseAttributes;
  std::map<int, std::vector<std::string> > spells;
  std::vector<std::string> equipment;
};

void load_classes();

PlayerClass& player_class_ref(const std::string& className);

#endif
