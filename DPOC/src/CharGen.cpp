#include "Config.h"
#include "Menu.h"
#include "Frame.h"
#include "Player.h"

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
    std::string currentChoice;

    switch (m_state)
    {
    case STATE_DEFAULT:
      currentChoice = getCurrentMenuChoice();

      if (currentChoice == "Add")
      {
        m_state = STATE_ADD;
      }
      else if (currentChoice == "Inspect")
      {
        m_state = STATE_INSPECT;
        m_characterMenu.setCursorVisible(true);
      }
      else if (currentChoice == "Remove")
      {
        m_state = STATE_REMOVE;
        m_characterMenu.setCursorVisible(true);
      }
      else if (currentChoice == "Done")
      {
      }

      break;
    case STATE_ADD:
      break;
    case STATE_INSPECT:
    case STATE_REMOVE:
      currentChoice = m_characterMenu.getCurrentMenuChoice();

      if (m_state == STATE_INSPECT)
      {
        // TODO: Draw summary.
      }
      else if (m_state == STATE_REMOVE)
      {
        get_player()->removeCharacter(currentChoice);
        m_characterMenu.refresh();
        m_state = STATE_DEFAULT;
        m_characterMenu.setCursorVisible(false);
      }

      break;
    }
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
    case STATE_REMOVE:
      m_characterMenu.moveArrow(dir);
      break;
    }
  }

  void draw(sf::RenderTarget& target, int x, int y)
  {
    //draw_frame(target, 0, 0, config::GAME_RES_X, config::GAME_RES_Y);

    m_characterMenu.draw(target, 0, 0);

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
