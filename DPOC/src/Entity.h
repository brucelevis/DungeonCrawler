#ifndef ENTITY_H
#define ENTITY_H

#include <string>

#include <SFML/Graphics.hpp>

#include "coord.h"

#include "Direction.h"
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

  void update();

  std::string getName() const { return m_name; }
  Sprite* sprite() { return m_sprite; }

  void setDirection(Direction dir);
  void step(Direction dir);

  void draw(sf::RenderTarget& target, const coord_t& view);

  float getRealX() const;
  float getRealY() const;
public:
  float x, y;
private:
  Entity();
  Entity(const Entity&);
  Entity& operator=(const Entity&);

  void walk();
private:
  std::string m_name;
  Sprite* m_sprite;

  Direction m_direction;
  float m_speed;
  float m_targetX, m_targetY;
  bool m_walking;
};

#endif
