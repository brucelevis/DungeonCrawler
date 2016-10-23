#ifndef MENU_SPELLMENU_H_
#define MENU_SPELLMENU_H_

#include <string>
#include <vector>
#include <functional>

#include "Range.h"
#include "Spell.h"
#include "GuiWidget.h"

class SpellMenu : public GuiWidget
{
public:
  using Callback = std::function<void(const Spell*)>;

  SpellMenu(const Callback& callback, const std::string& characterName, int x, int y);

  bool handleInput(sf::Keyboard::Key key) override;
  void draw(sf::RenderTarget& target) override;

  const Spell* getSelectedSpell() const;
  const Range& getRange() const { return m_range; }
  Range& getRange() { return m_range; }
private:
  int m_x, m_y;
  int m_width, m_height;
  Range m_range;

  std::vector<const Spell*> m_spells;

  Callback m_callback;
};

#endif /* MENU_SPELLMENU_H_ */
