#ifndef CHEST_H
#define CHEST_H

#include <vector>
#include <string>

#include "Entity.h"
#include "Trap.h"

class Chest : public Entity
{
public:
  Chest(const std::vector<std::string> items);
  Chest(const std::vector<std::string> items, const std::string& trapType, int luckToBeat);
  void update();
  void interact(const Entity* interactor);
private:
  void changeSprite();
  std::string getItemsString() const;
private:
  std::vector<std::string> m_items;
  bool m_spriteChanged;

  bool m_isTrapped;
  Trap m_trap;
};

#endif
