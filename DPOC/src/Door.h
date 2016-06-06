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
  enum DoorType
  {
    Door_OneWay,
    Door_TwoWay
  };

  Door(const std::string& name, const std::string& key, DoorType type = Door_OneWay);
  Door(const std::string& name, const std::string& key, const std::string& trapType, int luckToBeat, DoorType type = Door_OneWay);

  void update();
  void interact(const Entity*);

  bool isSeeThrough() const;

  bool isOpening() const;
  float getOpeningCount() const;

  DoorType getType() const { return m_type; }
  void open();
  void close();
private:
  void openFinished();
  bool isOpen() const;
  void closeFinished();
private:
  std::string m_keyRequired;

  bool m_isTrapped;
  Trap m_trap;

  DoorType m_type;
  DoorState m_state;

  float m_openingCount;

  // Ticks down when door is opened to close it again unless it was opened
  // with a key.
  int m_closeCounter;
};


#endif
