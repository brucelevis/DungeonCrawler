#ifndef CHEST_H
#define CHEST_H

#include <vector>
#include <string>

#include "Entity.h"

class Chest : public Entity
{
public:
  Chest(const std::vector<std::string> items);
  void update();
  void interact(const Entity* interactor);
private:
  void changeSprite();
  std::string getItemsString() const;
private:
  std::vector<std::string> m_items;
  bool m_spriteChanged;
};

#endif
