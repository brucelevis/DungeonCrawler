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

  virtual ~Character();

  std::string getName() const { return m_name; }

  static Character* createMonster(const std::string& name);

  const sf::Texture* getTexture() const { return m_faceTexture; }
  void draw(sf::RenderTarget& target, int x, int y) const;

  Attribute& getAttribute(const std::string& attribName);

  virtual int computeCurrentAttribute(const std::string& attribName);

  void setStatus(const std::string& status) { m_status = status; }
  std::string getStatus() const { return m_status; }
  void resetStatus();

  int spriteWidth() const { return m_textureRect.width; }
  int spriteHeight() const { return m_textureRect.height; }

  Flash& flash() { return m_flash; }

  bool incapacitated() const;

  void takeDamage(const std::string& attr, int amount);
protected:
  std::string m_name;

  sf::Texture* m_faceTexture;
  sf::IntRect m_textureRect;

  std::map<std::string, Attribute> m_attributes;

  std::string m_status;

  // Flash data
  Flash m_flash;
};

#endif
