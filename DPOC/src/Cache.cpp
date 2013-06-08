#include <map>

#include "logger.h"
#include "Cache.h"

namespace cache
{
  template <typename T>
  struct Entry
  {
    T* resource;
    int ref;
  };

  static std::map< std::string, Entry<sf::Texture> > textures;

  sf::Texture* loadTexture(const std::string& textureName)
  {
    auto it = textures.find(textureName);
    if (it == textures.end())
    {
      Entry<sf::Texture> newEntry;
      newEntry.resource = new sf::Texture;

      if (!newEntry.resource->loadFromFile(textureName))
      {
        TRACE("Unable to load texture: %s", textureName.c_str());
        delete newEntry.resource;
        return 0;
      }

      TRACE("Loaded texture: %s", textureName.c_str());

      newEntry.ref = 1;
      textures[textureName] = newEntry;

      return newEntry.resource;
    }
    else
    {
      sf::Texture* texture = it->second.resource;
      it->second.ref++;

      TRACE("Upping texture %s refCount, new refCount = %d", textureName.c_str(), it->second.ref);

      return texture;
    }
  }

  void releaseTexture(const std::string& textureName)
  {
    auto it = textures.find(textureName);
    if (it != textures.end())
    {
      it->second.ref--;

      TRACE("releaseTexture %s: refCount = %d", textureName.c_str(), it->second.ref);

      if (it->second.ref <= 0)
      {
        TRACE("Deleting texture: %s", textureName.c_str());

        delete it->second.resource;
        textures.erase(it);
      }
    }
    else
    {
      TRACE("Attempting to release texture %s that has not been previously loaded.", textureName.c_str());
    }
  }
}
