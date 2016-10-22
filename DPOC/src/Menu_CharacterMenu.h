#ifndef MENU_CHARACTERMENU_H_
#define MENU_CHARACTERMENU_H_

#include <string>
#include <functional>

#include "Spell.h"
#include "Range.h"
#include "GuiWidget.h"
#include "PlayerCharacter.h"

class CharacterMenu : public GuiWidget
{
public:
  using Callback = std::function<void(PlayerCharacter*)>;
  using EscapeCallback = std::function<void()>;

  CharacterMenu(const Callback& callback, const EscapeCallback& escapeCallback, int x, int y);

  void reset();

  bool handleInput(sf::Keyboard::Key key) override;
  void draw(sf::RenderTarget& target) override;

  void setSpellToUse(const Spell* spell) { m_spellToUse = spell; }
  const Spell* getSpellToUse() const { return m_spellToUse; }

  void setItemToUse(const std::string& itemToUse) { m_itemToUse = itemToUse; }
  std::string getItemToUse() const { return m_itemToUse; }

  PlayerCharacter* getUser() const { return m_user; }
  PlayerCharacter* getTarget() const { return m_target; }

  void setUser(PlayerCharacter* character);
  void setTarget(PlayerCharacter* character);
private:
  int m_x, m_y;

  const Spell* m_spellToUse;
  std::string m_itemToUse;

  PlayerCharacter* m_user;
  PlayerCharacter* m_target;

  std::vector<PlayerCharacter*> m_characters;
  Range m_range;

  Callback m_callback;
  EscapeCallback m_escapeCallback;
};

#endif /* MENU_CHARACTERMENU_H_ */
