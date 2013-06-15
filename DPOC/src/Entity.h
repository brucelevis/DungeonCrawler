#ifndef ENTITY_H
#define ENTITY_H

#include <string>

#include <SFML/Graphics.hpp>

#include "coord.h"

#include "Direction.h"
#include "Sprite.h"
#include "Script.h"

struct EntityDef
{
  std::string name;
  std::string spriteSheet;
  int spriteSheetX, spriteSheetY;
  std::string scriptFile;
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

  bool isWalking() const { return m_walking; }

  void interact(const Entity* interactor);
  void face(const Entity* entity);
  bool canInteractWith(const Entity* interactor) const;
public:
  float x, y;
private:
  Entity();
  Entity(const Entity&);
  Entity& operator=(const Entity&);

  void walk();
  void executeScriptLine(const Script::ScriptData& data);
private:
  std::string m_name;
  Sprite* m_sprite;

  Direction m_direction;
  float m_speed;
  float m_targetX, m_targetY;
  bool m_walking;

  Script m_script;
};

#endif
