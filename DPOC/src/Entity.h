#ifndef ENTITY_H
#define ENTITY_H

#include <string>

struct Entity
{
  std::string name;

  float x, y;
};

Entity* create_entity(const std::string& name);

#endif
