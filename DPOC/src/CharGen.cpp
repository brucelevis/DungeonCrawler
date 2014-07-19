#include <vector>
#include <cstdio>
#include <dirent.h>

#include "Cache.h"
#include "logger.h"
#include "draw_text.h"
#include "Sound.h"

#include "StatusEffect.h"
#include "Config.h"
#include "Menu.h"
#include "Frame.h"
#include "Player.h"
#include "PlayerClass.h"

#include "CharGen.h"

struct Proxy
{
  struct Listener
  {
    virtual ~Listener() {}
    virtual void textEntered(char c) = 0;
  };

  static Proxy& get()
  {
    static Proxy instance;
    return instance;
  }

  bool captureTyping;
  Listener* listener;

  void textEntered(char c)
  {
    if (listener)
    {
      listener->textEntered(c);
    }
  }

private:
  Proxy()
   : captureTyping(false),
     listener(0)
  {
  }
};

static std::vector<std::string> get_faces()
{
  std::vector<std::string> faces;
  DIR* dir = opendir(config::res_path("Faces").c_str());

  if (dir)
  {
    while (dirent* dir_entry = readdir(dir))
    {
      std::string face_name = dir_entry->d_name;

      if (face_name.find(".png") != std::string::npos)
      {
        faces.push_back(face_name);
      }
    }
    closedir(dir);
  }
  else
  {
    perror("opendir");
  }

  return faces;
}

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

  draw_text_bmp(target, x + 40 + 96, y + 12, "Lv: %d", character->computeCurrentAttribute("level"));
  draw_text_bmp(target, x + 40 + 96, y + 24, "Tn: %d", character->toNextLevel());

  y += 40;

  draw_text_bmp(target, x, y,      "Strength: %d", character->computeCurrentAttribute("strength"));
  draw_text_bmp(target, x, y + 12, "Defense:  %d", character->computeCurrentAttribute("defense"));
  draw_text_bmp(target, x, y + 24, "Magic:    %d", character->computeCurrentAttribute("magic"));
  draw_text_bmp(target, x, y + 36, "Mag.Def:  %d", character->computeCurrentAttribute("mag.def"));
  draw_text_bmp(target, x, y + 48, "Speed:    %d", character->computeCurrentAttribute("speed"));
  draw_text_bmp(target, x, y + 60, "Luck:     %d", character->computeCurrentAttribute("luck"));

  for (size_t i = 0; i < PlayerCharacter::equipNames.size(); i++)
  {
    Item* item = character->getEquipment(PlayerCharacter::equipNames[i]);
    draw_text_bmp(target, x, y + 84 + 12 * i, "%s: %s", PlayerCharacter::equipNames[i].c_str(), item ? item->name.c_str(): "");
  }
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

struct SelectFaceMenu : public Menu
{
  SelectFaceMenu()
   : m_faces(get_faces()),
     m_faceIndex(0),
     m_faceTexture(0),
     m_arrowTexture(cache::loadTexture("UI/Arrow.png"))
  {
    for (auto it = m_faces.begin(); it != m_faces.end(); ++it)
    {
      addEntry(*it);
    }

    reloadTexture();
  }

  ~SelectFaceMenu()
  {
    cache::releaseTexture(m_faceTexture);
    cache::releaseTexture(m_arrowTexture);
  }

  void handleConfirm()
  {
    setVisible(false);
  }

  void handleEscape()
  {
    Menu::handleEscape();
  }

  void moveArrow(Direction dir)
  {
    int oldIndex = m_faceIndex;

    if (dir == DIR_LEFT)
    {
      m_faceIndex--;
      if (m_faceIndex < 0) m_faceIndex = m_faces.size() - 1;
    }
    else if (dir == DIR_RIGHT)
    {
      m_faceIndex++;
      if ((size_t)m_faceIndex >= m_faces.size()) m_faceIndex = 0;
    }

    if (oldIndex != m_faceIndex)
    {
      reloadTexture();
    }

    setCurrentChoice(m_faceIndex);
  }

  void resetChoice()
  {
    m_faceIndex = 0;
    setCurrentChoice(m_faceIndex);
    reloadTexture();
  }

  void draw(sf::RenderTarget& target, int x, int y)
  {
    int width = 36;
    int height = 36;

    int xPos = config::GAME_RES_X / 2 - width / 2;
    int yPos = config::GAME_RES_Y / 2 - height / 2;

    draw_frame(target, xPos, yPos, width, height);

    sf::Sprite faceSprite;
    faceSprite.setTexture(*m_faceTexture);
    faceSprite.setPosition(xPos + 2, yPos + 2);
    target.draw(faceSprite);

    sf::Sprite arrowLeft;
    arrowLeft.setTexture(*m_arrowTexture);
    arrowLeft.setTextureRect(sf::IntRect(0, 0, 8, 8));
    arrowLeft.rotate(180);
    arrowLeft.setPosition(xPos - 2, yPos + height / 2 + 4);
    target.draw(arrowLeft);

    sf::Sprite arrowRight;
    arrowRight.setTextureRect(sf::IntRect(0, 0, 8, 8));
    arrowRight.setTexture(*m_arrowTexture);
    arrowRight.setPosition(xPos + width + 2, yPos + height / 2 - 4);
    target.draw(arrowRight);
  }
private:
  void reloadTexture()
  {
    if (m_faceTexture)
    {
      cache::releaseTexture(m_faceTexture);
    }

    m_faceTexture = cache::loadTexture("Faces/" + m_faces[m_faceIndex]);
  }
private:
  std::vector<std::string> m_faces;
  int m_faceIndex;
  sf::Texture* m_faceTexture;
  sf::Texture* m_arrowTexture;
};

struct GenerateMenu : public Menu, public Proxy::Listener
{
  static const size_t MAX_NAME_SIZE = 6;

  enum State
  {
    STATE_DEFAULT,
    STATE_ENTER_NAME,
    STATE_SELECT_CLASS,
    STATE_SELECT_AVATAR
  };

  GenerateMenu() :
    m_classes(get_all_classes()),
    m_selectClassMenu(m_classes),
    m_state(STATE_DEFAULT),
    m_faceTexture(0)
  {
    m_selectClassMenu.setVisible(false);

    addEntry("Name");
    addEntry("Class");
    addEntry("Avatar");
    addEntry("Done");
  }

  ~GenerateMenu()
  {
    if (m_faceTexture)
    {
      cache::releaseTexture(m_faceTexture);
    }
  }

  void handleConfirm()
  {
    if (m_state == STATE_DEFAULT)
    {
      std::string currentChoice = getCurrentMenuChoice();

      if (currentChoice == "Class")
      {
        m_selectClassMenu.setVisible(true);
        m_selectClassMenu.resetChoice();
        m_state = STATE_SELECT_CLASS;
      }
      else if (currentChoice == "Name")
      {
        m_state = STATE_ENTER_NAME;

        m_nameBuffer = "";
        Proxy::get().captureTyping = true;
        Proxy::get().listener = this;
      }
      else if (currentChoice == "Avatar")
      {
        m_selectFaceMenu.setVisible(true);
        m_selectFaceMenu.resetChoice();
        m_state = STATE_SELECT_AVATAR;
      }
      else if (currentChoice == "Done")
      {
        if (theName.size() && theClass.size() && theFace.size())
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
    else if (m_state == STATE_SELECT_AVATAR)
    {
      m_selectFaceMenu.handleConfirm();

      if (!m_selectFaceMenu.isVisible())
      {
        theFace = m_selectFaceMenu.getCurrentMenuChoice();

        reloadFace(theFace);

        m_state = STATE_DEFAULT;
      }
    }
  }

  void handleEscape()
  {
    if (m_state == STATE_DEFAULT)
    {
      setVisible(false);
      reset();

      resetChoice();
    }
    else if (m_state == STATE_SELECT_CLASS)
    {
      m_selectClassMenu.handleEscape();
      m_state = STATE_DEFAULT;
    }
    else if (m_state == STATE_ENTER_NAME)
    {
      clearProxy();
      m_state = STATE_DEFAULT;
    }
    else if (m_state == STATE_SELECT_AVATAR)
    {
      m_selectFaceMenu.handleEscape();
      m_state = STATE_DEFAULT;
    }
  }

  void moveArrow(Direction dir)
  {
    if (m_state == STATE_SELECT_CLASS)
    {
      m_selectClassMenu.moveArrow(dir);
    }
    else if (m_state == STATE_SELECT_AVATAR)
    {
      m_selectFaceMenu.moveArrow(dir);
    }
    else if (m_state == STATE_DEFAULT)
    {
      Menu::moveArrow(dir);
    }
  }

  void draw(sf::RenderTarget& target, int x, int y)
  {
    draw_frame(target, 0, 0, config::GAME_RES_X, config::GAME_RES_Y);

    Menu::draw(target, x, y);

    draw_frame(target, 6, 6, 36, 36, 1);
    if (m_faceTexture)
    {
      sf::Sprite sprite;
      sprite.setTexture(*m_faceTexture);
      sprite.setPosition(8, 8);
      target.draw(sprite);
    }

    draw_text_bmp(target, 44, 8,  "Name:  %s", theName.c_str());
    draw_text_bmp(target, 44, 20, "Class: %s", theClass.c_str());

    if (m_selectClassMenu.isVisible())
    {
      m_selectClassMenu.draw(target, x, y);
    }

    if (m_state == STATE_ENTER_NAME)
    {
      int width = MAX_NAME_SIZE * 8 + 16;
      int height = 16;

      int xPos = config::GAME_RES_X / 2 - width / 2;
      int yPos = config::GAME_RES_Y / 2 - height / 2;

      draw_frame(target, xPos, yPos, width, height);
      draw_text_bmp(target, xPos + 8, yPos + 4, "%s%s",
          m_nameBuffer.c_str(), m_nameBuffer.size() < MAX_NAME_SIZE ? "_" : "");
    }

    if (m_state == STATE_SELECT_AVATAR)
    {
      m_selectFaceMenu.draw(target, x, y);
    }
  }

  void textEntered(char c)
  {
    if (c == '\n' || c == '\r')
    {
      clearProxy();
      theName = m_nameBuffer;
      m_state = STATE_DEFAULT;
    }
    else if (c == '\b')
    {
      if (m_nameBuffer.size())
      {
        m_nameBuffer.resize(m_nameBuffer.size() - 1);
      }
    }
    else if (c == '\x1B')
    {
      clearProxy();
      m_state = STATE_DEFAULT;
    }
    else if (c == ' ')
    {
      // Fulhack
      if (m_nameBuffer.size())
      {
        m_nameBuffer += c;
      }
    }
    else if (c >= 32 && c <= 126)
    {
      if (m_nameBuffer.size() < MAX_NAME_SIZE)
      {
        m_nameBuffer += c;
      }
    }
  }

  void reset()
  {
    theClass = "";
    theName  = "";
    theFace  = "";

    if (m_faceTexture)
    {
      cache::releaseTexture(m_faceTexture);
      m_faceTexture = 0;
    }
  }

  std::string theClass;
  std::string theName;
  std::string theFace;
private:
  void clearProxy()
  {
    Proxy::get().captureTyping = false;
    Proxy::get().listener = 0;
  }

  void reloadFace(const std::string& name)
  {
    if (m_faceTexture)
    {
      cache::releaseTexture(m_faceTexture);
    }

    m_faceTexture = cache::loadTexture("Faces/" + theFace);
  }
private:
  std::vector<PlayerClass> m_classes;
  SelectClassMenu m_selectClassMenu;
  SelectFaceMenu  m_selectFaceMenu;
  State m_state;

  std::string m_nameBuffer;

  sf::Texture* m_faceTexture;
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
    draw_frame(target, 0, 0, config::GAME_RES_X, config::GAME_RES_Y);

    for (int i = 0; i < getNumberOfChoice(); i++)
    {
      PlayerCharacter* character = m_player->getCharacter(getChoice(i));

      static const int v_spacing = 40;

      int offX = 8;
      int offY = 8;

      character->draw(target, offX, offY + i * v_spacing);

      draw_text_bmp_ex(target, offX + 40, offY + i * v_spacing,
          get_status_effect(character->getStatus())->color,
          "%s (%s)", character->getName().c_str(), character->getStatus().c_str());
      draw_text_bmp(target, offX + 40, offY + i * v_spacing + 12, "Hp: %d/%d", character->getAttribute("hp").current, character->getAttribute("hp").max);
      draw_text_bmp(target, offX + 40, offY + i * v_spacing + 24, "Mp: %d/%d", character->getAttribute("mp").current, character->getAttribute("mp").max);

      if (cursorVisible() && getCurrentChoiceIndex() == i)
      {
        sf::RectangleShape rect = make_select_rect(offX - 2, offY + i * v_spacing - 2, 164, 36);
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
        if (m_player->getParty().size() < 4)
        {
          m_state = STATE_ADD;
          m_genMenu.setVisible(true);

          m_genMenu.resetChoice();
          m_genMenu.reset();
        }
        else
        {
          play_sound(config::get("SOUND_CANCEL"));
        }
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
        if (m_player->getCharacter(m_genMenu.theName))
        {
          // Disallow several characters with the same name.
          m_genMenu.setVisible(true);
          play_sound(config::get("SOUND_CANCEL"));
        }
        else
        {
          m_state = STATE_DEFAULT;

          if (m_genMenu.theClass.size() && m_genMenu.theName.size())
          {
            m_player->addNewCharacter(m_genMenu.theName, m_genMenu.theClass, "Faces/" + m_genMenu.theFace, 0, 0, 1);
            m_characterMenu.refresh();
          }
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
        m_characterMenu.resetChoice();

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
        m_characterMenu.handleEscape();
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

      if (m_state == STATE_INSPECT && m_inspectChar.size())
      {
        m_inspectChar = m_characterMenu.getCurrentMenuChoice();
      }
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
      draw_status(target, m_player->getCharacter(m_inspectChar), 24, 24);
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
  if (Proxy::get().captureTyping)
  {
    if (event.type == sf::Event::TextEntered)
    {
      sf::Uint32 unicode = event.text.unicode;

      if (unicode < 128)
      {
        Proxy::get().textEntered(static_cast<char>(unicode));
      }
    }
  }
  else
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
    else if (key == sf::Keyboard::Left) m_selectMenu->moveArrow(DIR_LEFT);
  }
}
