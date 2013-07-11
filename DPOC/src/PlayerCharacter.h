#ifndef PLAYER_CHARACTER_H
#define PLAYER_CHARACTER_H

#include <string>
#include <vector>

#include "Character.h"
#include "Item.h"
#include "PlayerClass.h"

struct CharacterData;

class PlayerCharacter : public Character
{
public:
  static std::vector<std::string> equipNames;

  PlayerCharacter();

  const std::vector<std::string>& getSpells() const { return m_spells; }

  void equip(const std::string& equipmentSlot, const std::string& itemName);
  Item* getEquipment(const std::string& equipmentSlot);
  bool canEquip(const std::string& itemName);

  int computeCurrentAttribute(const std::string& attribName);

  int toNextLevel();
  int expForLevel();
  int checkLevelUp(bool display = true);

  std::string xmlDump() const;

  static PlayerCharacter* create(const std::string& name);
  static PlayerCharacter* createFromSaveData(CharacterData* data);
private:
  void setAttributes();
  void setLevel(int levelReached, bool display = true);
private:
  std::map<std::string, Item> m_equipment;
  std::vector<std::string> m_spells;

  PlayerClass& m_class;
};

#endif
