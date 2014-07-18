#include <vector>

#include "draw_text.h"
#include "Sound.h"

#include "StatusEffect.h"
#include "Config.h"
#include "Menu.h"
#include "Frame.h"
#include "Player.h"
#include "PlayerClass.h"

#include "CharGen.h"

static sf::RectangleShape make_select_rect(int x, int y, int w, int h, sf::Color color = sf::Color::Red)
{
  sf::RectangleShape rect;
  rect.setFillColor(sf::Color::Transparent);
  rect.setOutlineThickness(1.0f);
  rect.setOutlineColor(color);
  rect.setSize(sf::Vector2f(w, h));
  rect.setPosition(x, y);

  return rect;
}

static void draw_status(sf::RenderTarget& target, PlayerCharacter* character, int x, int y)
{
  draw_frame(target, 16, 16, 14*16, 13*16);

  character->draw(target, x, y);

  draw_text_bmp_ex(target, x + 40, y,
      get_status_effect(character->getStatus())->color,
      "%s (%s)", character->getName().c_str(), character->getStatus().c_str());
  draw_text_bmp(target, x + 40, y + 12, "Hp: %d/%d", character->getAttribute("hp").current, character->getAttribute("hp").max);
  draw_text_bmp(target, x + 40, y + 24, "Mp: %d/%d", character->getAttribute("mp").current, character->getAttribute("mp").max);

//  draw_text_bmp(target, x + 40 + 96, y + 12, "Lv: %d", character->computeCurrentAttribute("level"));
//  draw_text_bmp(target, x + 40 + 96, y + 24, "Tn: %d", character->toNextLevel());

  y += 40;

  draw_text_bmp(target, x, y,      "Strength: %d", character->computeCurrentAttribute("strength"));
  draw_text_bmp(target, x, y + 12, "Defense:  %d", character->computeCurrentAttribute("defense"));
  draw_text_bmp(target, x, y + 24, "Magic:    %d", character->computeCurrentAttribute("magic"));
  draw_text_bmp(target, x, y + 36, "Mag.Def:  %d", character->computeCurrentAttribute("mag.def"));
  draw_text_bmp(target, x, y + 48, "Speed:    %d", character->computeCurrentAttribute("speed"));
  draw_text_bmp(target, x, y + 60, "Luck:     %d", character->computeCurrentAttribute("luck"));

//  for (size_t i = 0; i < PlayerCharacter::equipNames.size(); i++)
//  {
//    Item* item = character->getEquipment(PlayerCharacter::equipNames[i]);
//    draw_text_bmp(target, x, y + 84 + 12 * i, "%s: %s", PlayerCharacter::equipNames[i].c_str(), item ? item->name.c_str(): "");
//  }
}

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
    setVisible(false);
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
  enum State
  {
    STATE_DEFAULT,
    STATE_ENTER_NAME,
    STATE_SELECT_CLASS
  };

  GenerateMenu() :
    m_classes(get_all_classes()),
    m_selectClassMenu(m_classes),
    m_state(STATE_DEFAULT)
  {
    m_selectClassMenu.setVisible(false);

    addEntry("Enter name");
    addEntry("Select class");
    addEntry("Done");
  }

  void handleConfirm()
  {
    if (m_state == STATE_DEFAULT)
    {
      std::string currentChoice = getCurrentMenuChoice();

      if (currentChoice == "Select class")
      {
        m_selectClassMenu.setVisible(true);
        m_state = STATE_SELECT_CLASS;
      }
      else if (currentChoice == "Enter name")
      {

      }
      else if (currentChoice == "Done")
      {
        if (theName.size() && theClass.size())
        {
          setVisible(false);
        }
        else
        {
          play_sound(config::get("SOUND_CANCEL"));
        }
      }
    }
    else if (m_state == STATE_SELECT_CLASS)
    {
      m_selectClassMenu.handleConfirm();
      if (!m_selectClassMenu.isVisible())
      {
        theClass = m_selectClassMenu.getCurrentMenuChoice();
        m_state = STATE_DEFAULT;
      }
    }
  }

  void handleEscape()
  {
    if (m_state == STATE_DEFAULT)
    {
      setVisible(false);
      theName = "";
      theClass = "";
      resetChoice();
    }
    else if (m_state == STATE_SELECT_CLASS)
    {
      m_selectClassMenu.handleEscape();
      m_state = STATE_DEFAULT;
    }
    else if (m_state == STATE_ENTER_NAME)
    {

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
    draw_frame(target, 0, 0, config::GAME_RES_X, config::GAME_RES_Y);

    Menu::draw(target, x, y);

    draw_text_bmp(target, 8, 8,  "Name:  %s", theName.c_str());
    draw_text_bmp(target, 8, 20, "Class: %s", theClass.c_str());

    if (m_selectClassMenu.isVisible())
    {
      m_selectClassMenu.draw(target, x, y);
    }
  }

  std::string theClass;
  std::string theName;

private:
  std::vector<PlayerClass> m_classes;
  SelectClassMenu m_selectClassMenu;
  State m_state;
};

struct CharGenCharacterMenu : public Menu
{
  CharGenCharacterMenu(Player* player)
    : m_player(player)
  {
    setCursorVisible(false);
  }

  void handleConfirm()
  {
  }

  void handleEscape()
  {
    setCursorVisible(false);
    resetChoice();
  }

  int getWidth() const
  {
    return Menu::getWidth() + 32;
  }

  int getHeight() const
  {
    return (4 + 32) * getNumberOfChoice();
  }

  void refresh()
  {
    clear();

    const std::vector<PlayerCharacter*>& party = m_player->getParty();

    for (auto it = party.begin(); it != party.end(); ++it)
    {
      addEntry((*it)->getName());
    }
  }

  void draw(sf::RenderTarget& target, int x, int y)
  {
    draw_frame(target, x + 72, y, 184, 240);

    for (int i = 0; i < getNumberOfChoice(); i++)
    {
      PlayerCharacter* character = m_player->getCharacter(getChoice(i));

      int offX = x + 8 + 5 * 16;
      int offY = y + 8;

      character->draw(target, offX, offY + i * 48);

      draw_text_bmp_ex(target, offX + 40, offY + i * 48,
          get_status_effect(character->getStatus())->color,
          "%s (%s)", character->getName().c_str(), character->getStatus().c_str());
      draw_text_bmp(target, offX + 40, offY + i * 48 + 12, "Hp: %d/%d", character->getAttribute("hp").current, character->getAttribute("hp").max);
      draw_text_bmp(target, offX + 40, offY + i * 48 + 24, "Mp: %d/%d", character->getAttribute("mp").current, character->getAttribute("mp").max);

      if (cursorVisible() && getCurrentChoiceIndex() == i)
      {
        sf::RectangleShape rect = make_select_rect(offX - 2, offY + i * 48 - 2, 164, 36);
        target.draw(rect);
      }
    }
  }

private:
  Player* m_player;
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
     m_player(player),
     m_characterMenu(m_player)
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
          m_characterMenu.resetChoice();
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

      if (!m_genMenu.isVisible())
      {
        m_state = STATE_DEFAULT;

        if (m_genMenu.theClass.size() && m_genMenu.theName.size())
        {
          m_player->addNewCharacter(m_genMenu.theName, m_genMenu.theClass, 0, 0, 1);
          m_characterMenu.refresh();
        }
      }
      break;
    case STATE_INSPECT:
    case STATE_REMOVE:
      currentChoice = m_characterMenu.getCurrentMenuChoice();

      if (m_state == STATE_INSPECT)
      {
        m_inspectChar = currentChoice;
      }
      else if (m_state == STATE_REMOVE)
      {
        m_player->removeCharacter(currentChoice);
        m_characterMenu.refresh();

        if (m_player->getParty().empty())
        {
          m_state = STATE_DEFAULT;
          m_characterMenu.setCursorVisible(false);
        }
      }

      break;
    }
  }

  void handleEscape()
  {
    switch (m_state)
    {
    case STATE_DEFAULT:
      break;
    case STATE_ADD:
      m_genMenu.handleEscape();
      if (!m_genMenu.isVisible())
      {
        m_state = STATE_DEFAULT;
      }
      break;
    case STATE_INSPECT:
      if (m_inspectChar.empty())
      {
        m_state = STATE_DEFAULT;
      }
      else
      {
        m_inspectChar = "";
      }
      break;
    case STATE_REMOVE:
      m_characterMenu.handleEscape();
      m_state = STATE_DEFAULT;
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

    if (m_state == STATE_INSPECT && m_inspectChar.size())
    {
      draw_status(target, m_player->getCharacter(m_inspectChar), 8, 8);
    }
  }

private:
  State m_state;
  GenerateMenu m_genMenu;
  Player* m_player;
  CharGenCharacterMenu m_characterMenu;

  std::string m_inspectChar;
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
