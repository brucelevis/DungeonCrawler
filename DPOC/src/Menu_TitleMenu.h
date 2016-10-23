#ifndef MENU_TITLEMENU_H_
#define MENU_TITLEMENU_H_

#include <vector>
#include <functional>

#include "GuiWidget.h"
#include "MenuPresenter.h"

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
  Callback m_callback;
  MenuPresenter m_presenter;
};

#endif /* MENU_TITLEMENU_H_ */
