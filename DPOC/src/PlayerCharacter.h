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
  PlayerCharacter();
  ~PlayerCharacter();

  const std::vector<std::string>& getSpells() const { return m_spells; }

  void equip(const std::string& equipmentSlot, const std::string& itemName);
  Item* getEquipment(const std::string& equipmentSlot);
  bool canEquip(const std::string& itemName);

  int computeCurrentAttribute(const std::string& attribName);
  int getBaseAttribute(const std::string& attribName) const;
  void advanceAttribute(const std::string& attribName, int value);

  int toNextLevel();
  int expForLevel();
  int checkLevelUp(bool display = true);

  std::string xmlDump() const;

  /// Checks equipment for resistance.
  float getResistance(const std::string& element) const;

  bool isImmune(const std::string& status) const;

  const PlayerClass& getClass() const { return m_class; }

  void draw(sf::RenderTarget& target, int x, int y) const;

  static PlayerCharacter* create(const std::string& name, const std::string& className, int level = 1);
  static PlayerCharacter* create(const std::string& name, const std::string& className, const std::string& face, int level = 1);
  static PlayerCharacter* createFromSaveData(CharacterData* data);
private:
  void setAttributes();
  void setLevel(int levelReached, bool display = true);

  void setClass(const std::string& className);
private:
  std::map<std::string, Item> m_equipment;
  std::vector<std::string> m_spells;

  PlayerClass m_class;

  sf::Texture* m_skullTexture;
};

#endif
