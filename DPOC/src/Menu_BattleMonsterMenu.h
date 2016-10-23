#ifndef MENU_BATTLEMONSTERMENU_H_
#define MENU_BATTLEMONSTERMENU_H_

#include <vector>
#include <functional>

#include "Character.h"
#include "GuiWidget.h"

class BattleMonsterMenu : public GuiWidget
{
public:
  using Callback = std::function<void(Character*)>;

  BattleMonsterMenu(const Callback& callback, const std::vector<Character*>& monsters);

  bool handleInput(sf::Keyboard::Key key) override;
  void draw(sf::RenderTarget& target) override;

  Character* getCurrentMonster();

  /// Find first non-dead target.
  void fixSelection();

  void addMonster(Character* monster);
private:
  std::vector<Character*> m_monsters;
  Callback m_callback;
  int m_index;
};

#endif /* MENU_BATTLEMONSTERMENU_H_ */
