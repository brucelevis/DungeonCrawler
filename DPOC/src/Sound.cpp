#include <stdexcept>

#include <SFML/Audio.hpp>

#include "Cache.h"
#include "logger.h"
#include "Sound.h"

static std::vector<sf::Sound> activeSounds;

static void clear_stopped_sounds()
{
  for (auto it = activeSounds.begin(); it != activeSounds.end();)
  {
    if (it->getStatus() == sf::Sound::Stopped)
    {
      it = activeSounds.erase(it);
    }
    else
    {
      ++it;
    }
  }
}

void play_sound(const std::string& sndFile)
{
  clear_stopped_sounds();

  try
  {
    sf::SoundBuffer& buffer = cache::loadSound(sndFile);

    activeSounds.push_back(sf::Sound());
    activeSounds.back().setBuffer(buffer);
    activeSounds.back().play();
  }
  catch (std::runtime_error& e)
  {
    TRACE("play_sound: std::runtime_error(%s)", e.what());
  }
}

bool sound_is_playing()
{
  clear_stopped_sounds();

  return !activeSounds.empty();
}
