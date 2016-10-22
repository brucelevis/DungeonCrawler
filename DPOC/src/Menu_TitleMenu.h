#ifndef MENU_TITLEMENU_H_
#define MENU_TITLEMENU_H_

#include <vector>
#include <functional>

#include "Range.h"
#include "GuiWidget.h"

class TitleMenu : public GuiWidget
{
public:

  enum Action
  {
    NONE,
    NEW_GAME,
    LOAD_GAME,
    EXIT_GAME
  };

  using Callback = std::function<void(Action)>;

  TitleMenu(const Callback& callback);
  bool handleInput(sf::Keyboard::Key key) override;
  void draw(sf::RenderTarget& target) override;
private:
  void handleConfirm(const std::string& option);
  void gameLoaded();
private:
  std::vector<std::string> m_options;
  Range m_range;
  Callback m_callback;
};

#endif /* MENU_TITLEMENU_H_ */
