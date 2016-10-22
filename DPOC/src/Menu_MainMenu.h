#ifndef MENU_MAINMENU_H_
#define MENU_MAINMENU_H_

#include <vector>
#include <string>

#include "Range.h"
#include "GuiWidget.h"

struct Spell;
class PlayerCharacter;
class CharacterMenu;

class MainMenu : public GuiWidget
{
  enum State
  {
    STATE_DEFAULT,
    STATE_SELECT_SPELL,
    STATE_CAST_SPELL,
    STATE_VIEW_STATUS,
    STATE_VIEW_SKILLS,
    STATE_EQUIP,
    STATE_ITEM
  };
public:
  MainMenu();
  ~MainMenu();

  void start() override;
  bool handleInput(sf::Keyboard::Key key) override;
  void draw(sf::RenderTarget& target) override;
private:
  void handleConfirm();
  void addEntry(const std::string& entry);

  void activateCharacterMenu(State state);

  void characterSelected(PlayerCharacter* character);
  void itemSelected(const std::string& itemName);
  void spellSelected(const Spell* spell);

  void escapeFromCharacterMenu();
private:
  std::vector<std::string> m_options;
  Range m_range;
  CharacterMenu* m_characterMenu;
  State m_state;
};

#endif /* MENU_MAINMENU_H_ */
