#include <vector>
#include <string>
#include <algorithm>

#include "draw_text.h"
#include "Vocabulary.h"
#include "Character.h"
#include "MenuTextHelpers.h"

namespace
{
  int computeStatColumnLength()
  {
    std::vector<std::string> attrNames =
    {
      terms::strength,
      terms::defense,
      terms::magdef,
      terms::magic,
      terms::speed,
      terms::luck
    };

    auto maxElem = std::max_element(attrNames.begin(), attrNames.end());

    return static_cast<int>(maxElem->size()) + 1;
  }

  void print_attr(sf::RenderTarget& target, Character* character, const std::string& attrName, int x, int y, int maxColumnLength)
  {
    std::string toDraw = vocab(attrName) + ":";

    draw_text_bmp(target, x, y, "%-*s %d", maxColumnLength, toDraw.c_str(), character->computeCurrentAttribute(attrName));
  }
}

void draw_stat_block(sf::RenderTarget& target, Character* character, int x, int y)
{
  static int maxColumnLength = computeStatColumnLength();

  print_attr(target, character, terms::strength, x, y,      maxColumnLength);
  print_attr(target, character, terms::defense,  x, y + 12, maxColumnLength);
  print_attr(target, character, terms::magic,    x, y + 24, maxColumnLength);
  print_attr(target, character, terms::magdef,   x, y + 36, maxColumnLength);
  print_attr(target, character, terms::speed,    x, y + 48, maxColumnLength);
  print_attr(target, character, terms::luck,     x, y + 60, maxColumnLength);
}

void draw_hp(sf::RenderTarget& target, Character* character, int x, int y)
{
  draw_text_bmp(target, x, y, "%s: %d/%d", vocab(terms::hp).c_str(), character->getAttribute(terms::hp).current, character->getAttribute(terms::hp).max);
}

void draw_mp(sf::RenderTarget& target, Character* character, int x, int y)
{
  draw_text_bmp(target, x, y, "%s: %d/%d", vocab(terms::mp).c_str(), character->getAttribute(terms::mp).current, character->getAttribute(terms::mp).max);
}
