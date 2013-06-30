#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
#include <vector>
#include <stdexcept>

#include <SFML/Graphics.hpp>

#include "Item.h"
#include "Flash.h"

struct Attribute
{
  int current;
  int max;
};

class Character
{
public:
  static std::vector<std::string> equipNames;

  ~Character();

  std::string getName() const { return m_name; }
  const std::vector<std::string>& getSpells() const { return m_spells; }

  static Character* create(const std::string& name);
  static Character* createMonster(const std::string& name);

  const sf::Texture* getTexture() const { return m_faceTexture; }
  void draw(sf::RenderTarget& target, int x, int y) const;

  Attribute& getAttribute(const std::string& attribName);

  int computeCurrentAttribute(const std::string& attribName);

  void equip(const std::string& equipmentSlot, const std::string& itemName);
  Item* getEquipment(const std::string& equipmentSlot);

  std::string getStatus() const { return m_status; }

  int spriteWidth() const { return m_textureRect.width; }
  int spriteHeight() const { return m_textureRect.height; }

  Flash& flash() { return m_flash; }
private:
  std::string m_name;
  std::vector<std::string> m_spells;

  sf::Texture* m_faceTexture;
  sf::IntRect m_textureRect;

  std::map<std::string, Attribute> m_attributes;

  std::map<std::string, Item> m_equipment;

  std::string m_status;

  // Flash data
  Flash m_flash;
};

#endif
