#ifndef DOOR_H
#define DOOR_H

#include <string>

#include "Entity.h"
#include "Trap.h"

class Door : public Entity
{
public:
  Door(const std::string& key);
  Door(const std::string& key, const std::string& trapType, int luckToBeat);

  void update();
  void interact(const Entity*);
private:
  void open();
  bool isOpen() const;
private:
  std::string m_keyRequired;

  bool m_isTrapped;
  Trap m_trap;
};


#endif
