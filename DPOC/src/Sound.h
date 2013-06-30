#ifndef SOUND_H
#define SOUND_H

#include <SFML/Audio.hpp>

#include <string>

void play_sound(const std::string& sndFile);
bool sound_is_playing();

#endif
