#include "Frame.h"
#include "SaveMenu.h"
#include "draw_text.h"
#include "Utility.h"
#include "GuiStack.h"

#include "Menu_TitleMenu.h"

TitleMenu::TitleMenu(const Callback& callback)
  : m_callback(callback),
    m_presenter(MenuPresenter::STYLE_FRAME)
{
  m_presenter.addEntry("New Game");
  m_presenter.addEntry("Load Game");
  m_presenter.addEntry("Exit");
}

bool TitleMenu::handleInput(sf::Keyboard::Key key)
{
  switch (key)
  {
  case sf::Keyboard::Up:
    m_presenter.scrollUp();
    break;
  case sf::Keyboard::Down:
    m_presenter.scrollDown();
    break;
  case sf::Keyboard::Space:
  case sf::Keyboard::Return:
    handleConfirm(m_presenter.getSelectedOption().entryName);
    break;
  case sf::Keyboard::Escape:
    break;
  default:
    break;
  }

  return true;
}

void TitleMenu::handleConfirm(const std::string& option)
{
  if (option == "New Game")
  {
    m_callback(NEW_GAME);
  }
  else if (option == "Exit")
  {
    m_callback(EXIT_GAME);
  }
  else
  {
    auto loadMenu = getGuiStack()->addWidget<SaveMenu>(SaveMenu::LOAD);
    loadMenu->setLoadGameCallback(std::bind(&TitleMenu::gameLoaded, this));
  }
}

void TitleMenu::draw(sf::RenderTarget& target)
{
  const int x = target.getSize().x / 2 - m_presenter.getWidth() / 2;
  const int y = target.getSize().y / 2;

  m_presenter.draw(target, x, y, this);
}

void TitleMenu::gameLoaded()
{
  getGuiStack()->removeWidget(getGuiStack()->findWidget<SaveMenu>());
  m_callback(LOAD_GAME);
}
