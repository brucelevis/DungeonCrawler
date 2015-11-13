#ifndef MENUTEXTHELPERS_H_
#define MENUTEXTHELPERS_H_

#include <SFML/Graphics.hpp>

class Character;

void draw_stat_block(sf::RenderTarget& target, Character* character, int x, int y);

void draw_hp(sf::RenderTarget& target, Character* character, int x, int y);
void draw_mp(sf::RenderTarget& target, Character* character, int x, int y);

#endif /* MENUTEXTHELPERS_H_ */
