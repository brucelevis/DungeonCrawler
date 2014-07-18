#include <vector>

#include "draw_text.h"
#include "Sound.h"

#include "Config.h"
#include "Menu.h"
#include "Frame.h"
#include "Player.h"
#include "PlayerClass.h"

#include "CharGen.h"

struct SelectClassMenu : public Menu
{
  SelectClassMenu(const std::vector<PlayerClass>& classes)
   : m_classes(classes)
  {
    setMaxVisible(4);

    for (auto it = m_classes.begin(); it != m_classes.end(); ++it)
    {
      addEntry(it->name);
    }
  }

  void handleConfirm()
  {

  }

  void draw(sf::RenderTarget& target, int x, int y)
  {
    draw_frame(target, 0, 0, config::GAME_RES_X, config::GAME_RES_Y);

    PlayerClass currentClass = player_class_ref(getCurrentMenuChoice());
    draw_text_bmp(target, 8, 8, "%s", currentClass.description.c_str());

    int yPos = 72;

    draw_text_bmp(target, 8, yPos,  "Base attributes:");
    draw_text_bmp(target, 8, yPos + 12, "Strength: %d", currentClass.baseAttributes["strength"]);
    draw_text_bmp(target, 8, yPos + 24, "Defense:  %d", currentClass.baseAttributes["defense"]);
    draw_text_bmp(target, 8, yPos + 36, "Magic:    %d", currentClass.baseAttributes["magic"]);
    draw_text_bmp(target, 8, yPos + 48, "Mag.Def:  %d", currentClass.baseAttributes["mag.def"]);
    draw_text_bmp(target, 8, yPos + 60, "Speed:    %d", currentClass.baseAttributes["speed"]);
    draw_text_bmp(target, 8, yPos + 72, "Luck:     %d", currentClass.baseAttributes["luck"]);

    Menu::draw(target, 8, config::GAME_RES_Y - getHeight() - 8);
  }

private:
  const std::vector<PlayerClass>& m_classes;
};

struct GenerateMenu : public Menu
{
  GenerateMenu() :
   m_classes(get_all_classes()),
   m_selectClassMenu(m_classes)
  {
    m_selectClassMenu.setVisible(false);

    addEntry("Enter name");
    addEntry("Select class");
    addEntry("Done");
  }

  void handleConfirm()
  {
    if (getCurrentMenuChoice() == "Select class")
    {
      m_selectClassMenu.setVisible(true);
    }
  }

  void moveArrow(Direction dir)
  {
    if (m_selectClassMenu.isVisible())
    {
      m_selectClassMenu.moveArrow(dir);
    }
    else
    {
      Menu::moveArrow(dir);
    }
  }

  void draw(sf::RenderTarget& target, int x, int y)
  {
    Menu::draw(target, x, y);

    if (m_selectClassMenu.isVisible())
    {
      m_selectClassMenu.draw(target, x, y);
    }
  }

private:
  std::vector<PlayerClass> m_classes;
  SelectClassMenu m_selectClassMenu;
};

struct SelectMenu : public Menu
{
  enum State
  {
    STATE_DEFAULT,
    STATE_ADD,
    STATE_INSPECT,
    STATE_REMOVE
  };

  SelectMenu(Player* player)
   : m_state(STATE_DEFAULT),
     m_player(player)
  {
    addEntry("Add");
    addEntry("Inspect");
    addEntry("Remove");
    addEntry("Done");

    m_genMenu.setVisible(false);
    m_characterMenu.setCursorVisible(false);
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
        m_genMenu.setVisible(true);
      }
      else if (currentChoice == "Inspect")
      {
        if (m_player->getParty().size())
        {
          m_state = STATE_INSPECT;
          m_characterMenu.setCursorVisible(true);
        }
        else
        {
          play_sound(config::get("SOUND_CANCEL"));
        }
      }
      else if (currentChoice == "Remove")
      {
        if (m_player->getParty().size())
        {
          m_state = STATE_REMOVE;
          m_characterMenu.setCursorVisible(true);
        }
        else
        {
          play_sound(config::get("SOUND_CANCEL"));
        }
      }
      else if (currentChoice == "Done")
      {
      }

      break;
    case STATE_ADD:
      m_genMenu.handleConfirm();
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
      m_genMenu.moveArrow(dir);
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

    if (m_genMenu.isVisible())
    {
      m_genMenu.draw(target, x, y);
    }
    else
    {
      m_characterMenu.draw(target, 0, 0);

      Menu::draw(target, x, y);
    }
  }

private:
  State m_state;
  CharacterMenu m_characterMenu;
  GenerateMenu m_genMenu;
  Player* m_player;
};

CharGen::CharGen()
 : m_player(Player::createBlank()),
   m_selectMenu(new SelectMenu(m_player))
{
  m_selectMenu->setVisible(true);
}

CharGen::~CharGen()
{
  delete m_selectMenu;
}

void CharGen::update()
{

}

void CharGen::draw(sf::RenderTarget& target)
{
  m_selectMenu->draw(target, 8, config::GAME_RES_Y - m_selectMenu->getHeight() - 8);
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
  if (m_selectMenu->isVisible())
  {
    if (key == sf::Keyboard::Space)
    {
      m_selectMenu->handleConfirm();
    }
    else if (key == sf::Keyboard::Escape)
    {
      m_selectMenu->handleEscape();
    }

    if (key == sf::Keyboard::Down) m_selectMenu->moveArrow(DIR_DOWN);
    else if (key == sf::Keyboard::Up) m_selectMenu->moveArrow(DIR_UP);
    else if (key == sf::Keyboard::Right) m_selectMenu->moveArrow(DIR_RIGHT);
    else if (key == sf::Keyboard::Down) m_selectMenu->moveArrow(DIR_DOWN);
  }
}
