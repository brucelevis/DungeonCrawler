#ifndef MENU_BATTLEACTIONMENU_H_
#define MENU_BATTLEACTIONMENU_H_

#include <string>
#include <vector>
#include <functional>

#include "Range.h"
#include "GuiWidget.h"
#include "PlayerCharacter.h"

class BattleActionMenu : public GuiWidget
{
public:
  using ConfirmCallback = std::function<void(const std::string&)>;
  using EscapeCallback = std::function<void()>;

  BattleActionMenu(const ConfirmCallback& confirmCallback, const EscapeCallback& escapeCallback, int x, int y);

  bool handleInput(sf::Keyboard::Key key) override;
  void draw(sf::RenderTarget& target) override;

  void init(PlayerCharacter* character);

  void resetChoice();

  const std::string& getCurrentMenuChoice() const;
private:
  void addEntry(const std::string& option);
private:
  int m_x, m_y;

  Range m_range;
  std::vector<std::string> m_options;

  ConfirmCallback m_confirmCallback;
  EscapeCallback m_escapeCallback;
};

#endif /* MENU_BATTLEACTIONMENU_H_ */
