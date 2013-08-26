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
  Entity();
  Entity(const std::string& name);
  ~Entity();

  void update();

  std::string getName() const { return m_name; }
  Sprite* sprite() { return m_sprite; }
  Sprite* sprite() const { return m_sprite; }
  void setSprite(Sprite* sprite) { m_sprite = sprite; }

  void setDirection(Direction dir);
  Direction getDirection() const { return m_direction; }
  void step(Direction dir);

  void draw(sf::RenderTarget& target, const coord_t& view);

  void setPosition(int _x, int _y)
  {
    x = _x;
    y = _y;
    m_targetX = x;
    m_targetY = y;
  }

  float getRealX() const;
  float getRealY() const;

  int getTargetX() const { return m_targetX; }
  int getTargetY() const { return m_targetY; }

  bool isWalking() const { return m_state == STATE_WALKING; }

  void interact(const Entity* interactor);
  void face(const Entity* entity);
  bool canInteractWith(const Entity* interactor) const;

  void wait(int duration);

  void setTag(const std::string& tag) { m_tag = tag; }
  std::string getTag() const { return m_tag; }

  void setWalkThrough(bool walkthrough) { m_walkThrough = walkthrough; }
  void setWalkSpeed(float speed) { m_speed = speed; }

  void loadScripts(const std::string& talkScript, const std::string& stepScript);

  void setFixedDirection(bool fixed) { m_fixedDirection = fixed; }

  std::string xmlDump() const;

  void turnLeft();
  void turnRight();

  bool isVisible() const { return m_visible; }
public:
  float x, y;
private:
  Entity(const Entity&);
  Entity& operator=(const Entity&);

  void walk();
  void executeScriptLine(const Script::ScriptData& data, Script& executingScript);

  void getIfValue(const std::string& input, const std::string& key, int& value) const;

  bool checkPlayerCollision() const;
  bool checkEntityCollision() const;
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
  std::map<Script*, int> m_scriptWaitMap;

  std::string m_tag;

  bool m_walkThrough;
  bool m_visible;
  bool m_fixedDirection;
};

#endif
