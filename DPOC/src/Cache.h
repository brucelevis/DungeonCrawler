#ifndef CACHE_H
#define CACHE_H

#include <string>
#include <SFML/Graphics.hpp>

namespace cache
{
  sf::Texture* loadTexture(const std::string& textureName);
  void releaseTexture(const std::string& textureName);
}

#endif
