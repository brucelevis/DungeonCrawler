#include "Config.h"
#include "Menu.h"
#include "Frame.h"

#include "CharGen.h"

struct SelectMenu : public Menu
{
  enum State
  {
    STATE_DEFAULT,
    STATE_ADD,
    STATE_INSPECT,
    STATE_REMOVE
  };

  SelectMenu()
   : m_state(STATE_DEFAULT)
  {
    addEntry("Add");
    addEntry("Inspect");
    addEntry("Remove");
    addEntry("Done");
  }

  void handleConfirm()
  {

  }

  void moveArrow(Direction dir)
  {
    switch (m_state)
    {
    case STATE_DEFAULT:
      Menu::moveArrow(dir);
      break;
    case STATE_ADD:
      break;
    case STATE_INSPECT:
      break;
    case STATE_REMOVE:
      break;
    }
  }

  void draw(sf::RenderTarget& target, int x, int y)
  {
    //draw_frame(target, 0, 0, config::GAME_RES_X, config::GAME_RES_Y);

    Menu::draw(target, x, y);
  }

private:
  State m_state;
  CharacterMenu m_characterMenu;
};

void CharGen::update()
{

}

void CharGen::draw(sf::RenderTarget& target)
{

}

void CharGen::handleEvent(sf::Event& event)
{
  switch (event.type)
  {
  case sf::Event::KeyPressed:
    handleKeyPress(event.key.code);

    break;
  default:
    break;
  }
}

void CharGen::handleKeyPress(sf::Keyboard::Key key)
{
//  if (m_menu.isVisible())
//  {
//    if (key == sf::Keyboard::Space)
//    {
//      m_menu.handleConfirm();
//    }
//    else if (key == sf::Keyboard::Escape)
//    {
//      m_menu.handleEscape();
//    }
//
//    if (key == sf::Keyboard::Down) m_menu.moveArrow(DIR_DOWN);
//    else if (key == sf::Keyboard::Up) m_menu.moveArrow(DIR_UP);
//    else if (key == sf::Keyboard::Right) m_menu.moveArrow(DIR_RIGHT);
//    else if (key == sf::Keyboard::Down) m_menu.moveArrow(DIR_DOWN);
//  }
}
