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

  static Character* create(const std::string& name);
private:
  std::string m_name;
  std::vector<std::string> m_spells;

  sf::Texture* m_faceTexture;
};

#endif
