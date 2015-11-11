#ifndef DOOR_H
#define DOOR_H

#include <string>

#include "Entity.h"
#include "Trap.h"

class Door : public Entity
{
  enum DoorState
  {
    Door_Open,
    Door_Closed,
    Door_Opening,
    Door_Closing
  };
public:
  Door(const std::string& name, const std::string& key);
  Door(const std::string& name, const std::string& key, const std::string& trapType, int luckToBeat);

  void update();
  void interact(const Entity*);

  bool isMoving() const;

  bool isOpening() const;
  float getOpeningCount() const;
private:
  void open();
  void openFinished();
  bool isOpen() const;

  void close();
  void closeFinished();
private:
  std::string m_keyRequired;

  bool m_isTrapped;
  Trap m_trap;

  DoorState m_state;

  float m_openingCount;

  // Ticks down when door is opened to close it again unless it was opened
  // with a key.
  int m_closeCounter;
};


#endif
