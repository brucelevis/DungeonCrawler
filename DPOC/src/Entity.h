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
  float walkSpeed;
  std::string scriptFile;
  std::string stepScriptFile;
};

class Entity
{
  enum State
  {
    STATE_NORMAL,
    STATE_WAITING,
    STATE_WALKING
  };
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

  bool isWalking() const { return m_state == STATE_WALKING; }

  void interact(const Entity* interactor);
  void face(const Entity* entity);
  bool canInteractWith(const Entity* interactor) const;

  void wait(int duration);

  void setTag(const std::string& tag) { m_tag = tag; }
  std::string getTag() const { return m_tag; }
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

  // Run on interaction.
  Script m_script;
  // Run every update step.
  Script m_stepScript;

  State m_state;
  int m_waitCounter;

  std::string m_tag;
};

#endif
