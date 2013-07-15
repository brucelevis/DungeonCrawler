#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
#include <vector>
#include <stdexcept>

#include <SFML/Graphics.hpp>

#include "Item.h"
#include "Flash.h"

class StatusEffect;

struct Attribute
{
  int current;
  int max;
};

static inline void reset_attribute(Attribute& attr) { attr.current = attr.max; }
static inline void clamp_attribute(Attribute& attr)
{
  if (attr.current < 0)
    attr.current = 0;
  if (attr.current > attr.max)
    attr.current = attr.max;
}
static inline Attribute make_attribute(int val) { return { val, val }; }

class Character
{
public:
  Character();
  virtual ~Character();

  std::string getName() const { return m_name; }

  static Character* createMonster(const std::string& name);

  const sf::Texture* getTexture() const { return m_faceTexture; }
  void draw(sf::RenderTarget& target, int x, int y) const;

  Attribute& getAttribute(const std::string& attribName);

  virtual int computeCurrentAttribute(const std::string& attribName);

  /// @return True if status was afflicted on character.
  bool afflictStatus(const std::string& status, int duration);

  /// @return True if status was cured from character.
  bool cureStatus(const std::string& status);

  bool hasStatus(const std::string& status);
  std::string getStatus() const;
  void resetStatus();
  bool tickStatusDurations();

  const std::vector<StatusEffect*> getStatusEffects() const { return m_status; }

  int spriteWidth() const { return m_textureRect.width; }
  int spriteHeight() const { return m_textureRect.height; }

  Flash& flash() { return m_flash; }

  bool incapacitated() const;

  void takeDamage(const std::string& attr, int amount);

  virtual float getResistance(const std::string& element) const;
  virtual bool isImmune(const std::string& status) const;
private:
  std::vector<StatusEffect*>::iterator getStatusEffectIterator(const std::string& status);
protected:
  std::string m_name;

  sf::Texture* m_faceTexture;
  sf::IntRect m_textureRect;
  sf::Color m_color;

  std::map<std::string, Attribute> m_attributes;

  std::vector<StatusEffect*> m_status;
  std::map<StatusEffect*, int> m_statusDurations;

  std::map<std::string, float> m_resistance;
  std::vector<std::string> m_statusImmunity;

  // Flash data
  Flash m_flash;
};

#endif
