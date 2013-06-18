#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

class Character
{
public:
  ~Character();

  std::string getName() const { return m_name; }
  const std::vector<std::string>& getSpells() const { return m_spells; }

  static Character* create(const std::string& name);

  const sf::Texture* getTexture() const { return m_faceTexture; }
private:
  std::string m_name;
  std::vector<std::string> m_spells;

  sf::Texture* m_faceTexture;
};

#endif
