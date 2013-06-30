#ifndef CACHE_H
#define CACHE_H

#include <string>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

namespace cache
{
  sf::Texture* loadTexture(const std::string& textureName);
  void releaseTexture(const std::string& textureName);
  void releaseTexture(sf::Texture* texture);

  sf::SoundBuffer& loadSound(const std::string& sndFile);
}

#endif
