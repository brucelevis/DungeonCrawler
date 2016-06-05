#include <vector>
#include <cstdio>
#include <dirent.h>

#include "Cache.h"
#include "logger.h"
#include "draw_text.h"
#include "Sound.h"

#include "Game.h"
#include "SceneManager.h"

#include "Vocabulary.h"
#include "Item.h"
#include "StatusEffect.h"
#include "Config.h"
#include "Menu.h"
#include "Frame.h"
#include "Player.h"
#include "PlayerClass.h"
#include "MenuTextHelpers.h"
#include "Utility.h"
#include "Error.h"

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

  draw_hp(target, character, x + 40, y + 12);
  draw_mp(target, character, x + 40, y + 24);

  draw_text_bmp(target, x + 40 + 96, y + 12, "Lv: %d", character->computeCurrentAttribute(terms::level));
  draw_text_bmp(target, x + 40 + 96, y + 24, "Tn: %d", character->toNextLevel());

  y += 40;

  draw_stat_block(target, character, x, y);

  const auto equipNames = get_equip_names();
  for (size_t i = 0; i < equipNames.size(); i++)
  {
    Item* item = character->getEquipment(equipNames[i]);
    draw_text_bmp(target, x, y + 84 + 12 * i, "%s: %s", vocab(equipNames[i]).c_str(), item ? item->name.c_str(): "");
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
    (void)x;
    (void)y;

    draw_frame(target, 0, 0, config::GAME_RES_X, config::GAME_RES_Y);

    PlayerClass currentClass = player_class_ref(getCurrentMenuChoice());
    draw_text_bmp(target, 8, 8, "%s", currentClass.description.c_str());

    int yPos = 76;

    draw_text_bmp(target, 8, yPos,  "Base attributes:");
    draw_stat_block(target, currentClass, 8, yPos + 12);

    Menu::draw(target, 8, config::GAME_RES_Y - getHeight() - 8);
  }

private:
  const std::vector<PlayerClass>& m_classes;
};

struct SelectFaceMenu : public Menu
{
  const int EyesW = 32;
  const int EyesH = 16;
  const int FaceW = 32;
  const int FaceH = 32;

  enum EditMode
  {
    MODE_HEAD     = 0,
    MODE_EYE      = 1,
    MODE_EYEBROW  = 2,
    MODE_BEARD    = 3,
    MODE_HAIR     = 4,
    MODE_NOSE     = 5,
    MODE_MOUTH    = 6,
    MODE_EAR      = 7
  };

  SelectFaceMenu()
   : m_arrowTexture(cache::loadTexture("UI/Arrow.png")),

     m_heads(cache::loadTexture("Customize/Heads.png")),
     m_ears(cache::loadTexture("Customize/Ears.png")),
     m_eyes(cache::loadTexture("Customize/Eyes.png")),
     m_eyebrows(cache::loadTexture("Customize/Eyebrows.png")),
     m_beards(cache::loadTexture("Customize/Beards.png")),
     m_hairStyles(cache::loadTexture("Customize/Hair.png")),
     m_noses(cache::loadTexture("Customize/Noses.png")),
     m_mouths(cache::loadTexture("Customize/Mouths.png")),

     m_editMode(MODE_HEAD)
  {
    m_numberOfHeads        = (m_heads->getSize().x / FaceW) * (m_heads->getSize().y / FaceH);
    m_numberOfEar          = (m_ears->getSize().x / FaceW) * (m_ears->getSize().y / FaceH);
    m_numberOfEyes         = (m_eyes->getSize().x / FaceW) * (m_eyes->getSize().y / EyesH);
    m_numberOfEyeBrows     = (m_eyebrows->getSize().x / FaceW) * (m_eyebrows->getSize().y / EyesH);
    m_numberOfBeards       = (m_beards->getSize().x / FaceW) * (m_beards->getSize().y / EyesH);
    m_numberOfHair         = (m_hairStyles->getSize().x / FaceW) * (m_hairStyles->getSize().y / FaceH);
    m_numberOfNose         = (m_noses->getSize().x / FaceW) * (m_noses->getSize().y / EyesH);
    m_numberOfMouth        = (m_mouths->getSize().x / FaceW) * (m_mouths->getSize().y / EyesH);
  }

  ~SelectFaceMenu()
  {
    cache::releaseTexture(m_arrowTexture);

    cache::releaseTexture(m_heads);
    cache::releaseTexture(m_ears);
    cache::releaseTexture(m_eyes);
    cache::releaseTexture(m_eyebrows);
    cache::releaseTexture(m_beards);
    cache::releaseTexture(m_hairStyles);
    cache::releaseTexture(m_noses);
    cache::releaseTexture(m_mouths);
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
    if (dir == DIR_LEFT)
    {
      (*m_currentIndex)--;
      if ((*m_currentIndex) < 0) (*m_currentIndex) = (*m_currentMax) - 1;
    }
    else if (dir == DIR_RIGHT)
    {
      (*m_currentIndex)++;
      if ((*m_currentIndex) >= (*m_currentMax)) (*m_currentIndex) = 0;
    }
    else if (dir == DIR_UP)
    {
      if (static_cast<int>(m_editMode) != static_cast<int>(MODE_HEAD))
      {
        m_editMode = static_cast<EditMode>( static_cast<int>(m_editMode) - 1 );
        setupPointers();
      }
    }
    else if (dir == DIR_DOWN)
    {
      if (static_cast<int>(m_editMode) != static_cast<int>(MODE_EAR))
      {
        m_editMode = static_cast<EditMode>( static_cast<int>(m_editMode) + 1 );
        setupPointers();
      }
    }
  }

  void resetChoice()
  {
    m_currentHead     = 0;
    m_currentEar      = 0;
    m_currentEye      = 0;
    m_currentEyeBrows = 0;
    m_currentBeard    = 0;
    m_currentHair     = 0;
    m_currentNose     = 0;
    m_currentMouth    = 0;

    m_currentIndex = &m_currentHead;
    m_currentMax   = &m_numberOfHeads;
    m_editMode = MODE_HEAD;
  }

  void draw(sf::RenderTarget& target, int x, int y)
  {
    (void)x;
    (void)y;

    int width = 36;
    int height = 36;

    int xPos = config::GAME_RES_X / 2 - width / 2;
    int yPos = config::GAME_RES_Y / 2 - height / 2;

    drawEditMode(target, xPos, yPos - 16);

    draw_frame(target, xPos, yPos, width, height);

    int drawPosX = xPos + 2;
    int drawPosY = yPos + 2;
    drawFace(target, drawPosX, drawPosY);

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

  void randomize()
  {
    m_currentHead     = random_range(0, m_numberOfHeads - 1);
    m_currentEar      = random_range(0, m_numberOfEar - 1);
    m_currentEye      = random_range(0, m_numberOfEyes - 1);
    m_currentEyeBrows = random_range(0, m_numberOfEyeBrows - 1);
    m_currentBeard    = random_range(0, m_numberOfBeards - 1);
    m_currentHair     = random_range(0, m_numberOfHair - 1);
    m_currentNose     = random_range(0, m_numberOfNose - 1);
    m_currentMouth    = random_range(0, m_numberOfMouth - 1);
  }

  std::string saveFace()
  {
    std::vector<int> indices{m_currentHead, m_currentEar, m_currentEye, m_currentEyeBrows, m_currentBeard, m_currentHair, m_currentNose, m_currentMouth};

    std::string hashString;
    for (const auto& i : indices)
    {
      hashString += toString(i);
    }

    hashString += ".png";
    TRACE("Saving face %s.", hashString.c_str());

    sf::RenderTexture texture;
    texture.create(FaceW, FaceH);
    texture.clear(sf::Color::Transparent);
    drawFace(texture, 0, 0);

    texture.display();
    sf::Image imageToSave = texture.getTexture().copyToImage();
    if (imageToSave.saveToFile(config::res_path("Faces/" + hashString)))
    {
      TRACE("Image successfully saved.");
    }
    else
    {
      CRASH("Failed to save %s!!!", hashString.c_str());
    }

    return hashString;
  }
private:
  void drawFace(sf::RenderTarget& target, int drawPosX, int drawPosY) const
  {
    drawPart(target, m_heads, m_currentHead, FaceW, FaceH, drawPosX, drawPosY);
    drawPart(target, m_ears,  m_currentEar,  FaceW, FaceH, drawPosX, drawPosY);
    drawPart(target, m_eyes,  m_currentEye, EyesW, EyesH, drawPosX, drawPosY + FaceH / 2 - EyesH / 2);
    drawPart(target, m_eyebrows, m_currentEyeBrows, EyesW, EyesH, drawPosX, drawPosY);
    drawPart(target, m_beards, m_currentBeard, EyesW, EyesH, drawPosX, drawPosY + FaceH / 2);
    drawPart(target, m_noses, m_currentNose, EyesW, EyesH, drawPosX, drawPosY + FaceH / 2 - EyesH / 2);
    drawPart(target, m_mouths, m_currentMouth, EyesW, EyesH, drawPosX, drawPosY + FaceH / 2);
    drawPart(target, m_hairStyles, m_currentHair, FaceW, FaceH, drawPosX, drawPosY);
  }

  void drawEditMode(sf::RenderTarget& target, int x, int y) const
  {
    std::string stringToDraw;

    switch (m_editMode)
    {
    case MODE_HEAD:
      stringToDraw = "Choose head";
      break;
    case MODE_EYE:
      stringToDraw = "Choose eyes";
      break;
    case MODE_EYEBROW:
      stringToDraw = "Choose eyebrows";
      break;
    case MODE_BEARD:
      stringToDraw = "Choose beard";
      break;
    case MODE_HAIR:
      stringToDraw = "Choose hair";
      break;
    case MODE_NOSE:
      stringToDraw = "Choose nose";
      break;
    case MODE_MOUTH:
      stringToDraw = "Choose mouth";
      break;
    case MODE_EAR:
      stringToDraw = "Choose ears";
      break;
    }

    static const std::string randomStr = "Press R to randomize";

    draw_text_bmp(target, x - 8 * randomStr.size() / 2, y - 8, "%s", randomStr.c_str());
    draw_text_bmp(target, x - 8 * (stringToDraw.size() / 2), y, "%s (%d/%d)",
      stringToDraw.c_str(), (*m_currentIndex) + 1, *m_currentMax);
  }

  void setupPointers()
  {
    switch (m_editMode)
    {
    case MODE_HEAD:
      m_currentIndex = &m_currentHead;
      m_currentMax   = &m_numberOfHeads;
      break;
    case MODE_EYE:
      m_currentIndex = &m_currentEye;
      m_currentMax   = &m_numberOfEyes;
      break;
    case MODE_EYEBROW:
      m_currentIndex = &m_currentEyeBrows;
      m_currentMax   = &m_numberOfEyeBrows;
      break;
    case MODE_BEARD:
      m_currentIndex = &m_currentBeard;
      m_currentMax   = &m_numberOfBeards;
      break;
    case MODE_HAIR:
      m_currentIndex = &m_currentHair;
      m_currentMax   = &m_numberOfHair;
      break;
    case MODE_NOSE:
      m_currentIndex = &m_currentNose;
      m_currentMax   = &m_numberOfNose;
      break;
    case MODE_MOUTH:
      m_currentIndex = &m_currentMouth;
      m_currentMax   = &m_numberOfMouth;
      break;
    case MODE_EAR:
      m_currentIndex = &m_currentEar;
      m_currentMax   = &m_numberOfEar;
      break;
    }
  }

  void drawPart(sf::RenderTarget& target, sf::Texture* textureToUse, int index, int dimX, int dimY, int posX, int posY) const
  {
    int amountX = textureToUse->getSize().x / dimX;

    int selX = (index % amountX) * dimX;
    int selY = (index / amountX) * dimY;

    sf::Sprite sprite;
    sprite.setPosition(posX, posY);
    sprite.setTexture(*textureToUse);
    sprite.setTextureRect(sf::IntRect{selX, selY, dimX, dimY});

    target.draw(sprite);
  }
private:
  sf::Texture* m_arrowTexture;

  sf::Texture* m_heads;
  sf::Texture* m_ears;
  sf::Texture* m_eyes;
  sf::Texture* m_eyebrows;
  sf::Texture* m_beards;
  sf::Texture* m_hairStyles;
  sf::Texture* m_noses;
  sf::Texture* m_mouths;

  EditMode m_editMode;

  int m_currentHead     = 0;
  int m_currentEar      = 0;
  int m_currentEye      = 0;
  int m_currentEyeBrows = 0;
  int m_currentBeard    = 0;
  int m_currentHair     = 0;
  int m_currentNose     = 0;
  int m_currentMouth    = 0;

  int m_numberOfHeads;
  int m_numberOfEar;
  int m_numberOfEyes;
  int m_numberOfEyeBrows;
  int m_numberOfBeards;
  int m_numberOfHair;
  int m_numberOfNose;
  int m_numberOfMouth;

  int* m_currentIndex = &m_currentHead;
  int* m_currentMax   = &m_numberOfHeads;
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
        theFace = m_selectFaceMenu.saveFace();

        reloadFace();

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
      int height = 24;

      int xPos = config::GAME_RES_X / 2 - width / 2;
      int yPos = config::GAME_RES_Y / 2 - height / 2;

      draw_frame(target, xPos, yPos, width, height);
      draw_text_bmp(target, xPos + 8, yPos + 8, "%s%s",
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

  void randomizeFace()
  {
    if (m_state == STATE_SELECT_AVATAR)
    {
      m_selectFaceMenu.randomize();
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

  void reloadFace()
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
    (void)x;
    (void)y;

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

      draw_hp(target, character, offX + 40, offY + i * v_spacing + 12);
      draw_mp(target, character, offX + 40, offY + i * v_spacing + 24);

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
        if (m_player->getParty().size() == 4)
        {
          setVisible(false);
          SceneManager::instance().fadeOut(32);
        }
        else
        {
          play_sound(config::get("SOUND_CANCEL"));
        }
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
            defaultEquip();
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

  void randomizeFace()
  {
    m_genMenu.randomizeFace();
  }
private:
  void defaultEquip()
  {
    PlayerCharacter* character = m_player->getCharacter(m_genMenu.theName);

    PlayerClass playerClass = character->getClass();
    for (auto it = playerClass.startingEquipment.begin(); it != playerClass.startingEquipment.end(); ++it)
    {
      auto item = item_ref(*it);
      std::string equipType = equip_type_string(item.type);

      character->equip(equipType, item.name);
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
    else if (key == sf::Keyboard::R)
    {
      dynamic_cast<SelectMenu*>(m_selectMenu)->randomizeFace();
    }

    if (key == sf::Keyboard::Down) m_selectMenu->moveArrow(DIR_DOWN);
    else if (key == sf::Keyboard::Up) m_selectMenu->moveArrow(DIR_UP);
    else if (key == sf::Keyboard::Right) m_selectMenu->moveArrow(DIR_RIGHT);
    else if (key == sf::Keyboard::Left) m_selectMenu->moveArrow(DIR_LEFT);
  }
}

void CharGen::postFade(FadeType fadeType)
{
  if (fadeType == FADE_OUT)
  {
    close();
    //m_titleMusic.stop();

    SceneManager::instance().fadeIn(32);

    Game::instance().start(m_player);
    SceneManager::instance().addScene(&Game::instance());
  }
}
