#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include "Sprite.h"

struct EntityDef
{
  std::string name;
  std::string spriteSheet;
  int spriteSheetX, spriteSheetY;
};

class Entity
{
public:
  Entity(const std::string& name);
  ~Entity();

  std::string getName() const { return m_name; }
  Sprite* sprite() { return m_sprite; }
public:
  float x, y;
private:
  Entity();
  Entity(const Entity&);
  Entity& operator=(const Entity&);

  std::string m_name;
  Sprite* m_sprite;
};

#endif
