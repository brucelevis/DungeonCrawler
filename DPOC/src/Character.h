#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

struct Attribute
{
  int current;
  int max;
};

class Character
{
public:
  ~Character();

  std::string getName() const { return m_name; }
  const std::vector<std::string>& getSpells() const { return m_spells; }

  static Character* create(const std::string& name);

  const sf::Texture* getTexture() const { return m_faceTexture; }

  Attribute getAttribute(const std::string& attribName);
private:
  std::string m_name;
  std::vector<std::string> m_spells;

  sf::Texture* m_faceTexture;

  std::map<std::string, Attribute> m_attributes;
};

#endif
