#ifndef PLAYER_CHARACTER_H
#define PLAYER_CHARACTER_H

#include <string>
#include <vector>

#include "Character.h"
#include "Item.h"

class PlayerCharacter : public Character
{
public:
  static std::vector<std::string> equipNames;

  const std::vector<std::string>& getSpells() const { return m_spells; }

  void equip(const std::string& equipmentSlot, const std::string& itemName);
  Item* getEquipment(const std::string& equipmentSlot);

  int computeCurrentAttribute(const std::string& attribName);

  int toNextLevel();
  int expForLevel();
  int checkLevelUp();

  static PlayerCharacter* create(const std::string& name);
private:
  std::map<std::string, Item> m_equipment;
  std::vector<std::string> m_spells;
};

#endif
