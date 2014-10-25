#include "Sound.h"
#include "Utility.h"
#include "draw_text.h"
#include "Frame.h"
#include "Player.h"
#include "PlayerCharacter.h"
#include "SceneManager.h"
#include "Config.h"
#include "Persistent.h"
#include "SkillTrainer.h"

class TrainMenu : public Menu
{
public:
  TrainMenu(const std::vector<std::string>& skills, int height, PlayerCharacter* currentCharacter)
   : m_skills(skills),
     m_height(height - 20),
     m_currentCharacter(currentCharacter)
  {
    setMaxVisible(m_height / 12);

    updateEntries();
  }

  void handleConfirm()
  {
    int gems = getRequiredGems();

    if (global<int>("$sys:gems") >= gems)
    {
      set_global("$sys:gems", global<int>("$sys:gems") - gems);
      m_currentCharacter->advanceAttribute(getCurrentSkill(), 1);

      play_sound(config::get("SOUND_SHOP"));

      updateEntries();
    }
    else
    {
      play_sound(config::get("SOUND_CANCEL"));
    }
  }

  int getWidth() const
  {
    return config::GAME_RES_X;
  }

  int getHeight() const
  {
    return m_height;
  }

  void draw(sf::RenderTarget& target, int x, int y)
  {
    draw_frame(target, 0, 0, config::GAME_RES_X, 24);
    draw_text_bmp(target, 8, 8, "Gems to train: %d / %d", getRequiredGems(), global<int>("$sys:gems"));

    Menu::draw(target, x, y + 22);
  }
private:
  int getRequiredGems() const
  {
    std::string skill = getCurrentSkill();

    if (skill.size())
    {
      return m_currentCharacter->getBaseAttribute(skill) + 1;
    }

    return -1;
  }

  std::string getCurrentSkill() const
  {
    auto choiceVec = split_string(getCurrentMenuChoice(), ' ');
    if (choiceVec.size())
    {
      return choiceVec.front();
    }

    return "";
  }

  void updateEntries()
  {
    clear();

    for (const std::string& skill : m_skills)
    {
      // Add char skill % to entry as well as space padding to line stuff up
      // nicely.
      std::string percent = toString(m_currentCharacter->getBaseAttribute(skill)) + "%";
      int length = skill.size() + percent.size();

      int spacePad = getWidth() / 8 - length - 3;
      std::string entry = skill;
      for (int i = 0; i < spacePad; i++)
      {
        entry += ' ';
      }
      entry += percent;

      addEntry(entry);
    }
  }
private:
  std::vector<std::string> m_skills;
  int m_height;
  PlayerCharacter* m_currentCharacter;
};

SkillTrainer::SkillTrainer(const std::vector<std::string>& skills)
 : m_skills(skills),
   m_trainMenu(nullptr)
{
  for (PlayerCharacter* pc : get_player()->getParty())
  {
    m_choiceMenu.addEntry(pc->getName());
  }
  m_choiceMenu.addEntry("Leave");
}

SkillTrainer::~SkillTrainer()
{
  delete m_trainMenu;
}

void SkillTrainer::update()
{

}

void SkillTrainer::draw(sf::RenderTarget& target)
{
  draw_frame(target, 0, 0, config::GAME_RES_X, config::GAME_RES_Y);

  int y = config::GAME_RES_Y - m_choiceMenu.getHeight();

  m_choiceMenu.draw(target, 0, y);

  for (size_t i = 0; i < get_player()->getParty().size(); i++)
  {
    PlayerCharacter* pc = get_player()->getParty()[i];

    sf::Sprite sprite;
    sprite.setTexture(*pc->getTexture());
    sprite.setPosition(m_choiceMenu.getWidth() + 16 + i * 34,
        y + pc->getTexture()->getSize().y / 2);

    if (pc != getCurrentCharacter())
    {
      sprite.setColor(sf::Color(48, 48, 48));
    }

    target.draw(sprite);
  }

  if (m_trainMenu && m_trainMenu->isVisible())
  {
    m_trainMenu->draw(target, 0, 0);
  }
}

void SkillTrainer::handleEvent(sf::Event& event)
{
  if (event.type == sf::Event::KeyPressed)
  {
    handleKeyPress(event.key.code);
  }
}

void SkillTrainer::handleKeyPress(sf::Keyboard::Key key)
{
  if (key == sf::Keyboard::Down)
  {
    if (m_trainMenu && m_trainMenu->isVisible())
    {
      m_trainMenu->moveArrow(DIR_DOWN);
    }
    else
    {
      m_choiceMenu.moveArrow(DIR_DOWN);
    }
  }
  else if (key == sf::Keyboard::Up)
  {
    if (m_trainMenu && m_trainMenu->isVisible())
    {
      m_trainMenu->moveArrow(DIR_UP);
    }
    else
    {
      m_choiceMenu.moveArrow(DIR_UP);
    }
  }
  else if (key == sf::Keyboard::Space)
  {
    if (m_trainMenu && m_trainMenu->isVisible())
    {
      m_trainMenu->handleConfirm();
    }
    else
    {
      if (m_choiceMenu.getCurrentMenuChoice() == "Leave")
      {
        escape();
      }
      else
      {
        m_trainMenu = new TrainMenu(m_skills, config::GAME_RES_Y - m_choiceMenu.getHeight(), getCurrentCharacter());
        m_trainMenu->setVisible(true);
        m_choiceMenu.setCursorVisible(false);
      }
    }
  }
  else if (key == sf::Keyboard::Escape)
  {
    if (m_trainMenu && m_trainMenu->isVisible())
    {
      m_trainMenu->handleEscape();
      if (!m_trainMenu->isVisible())
      {
        delete m_trainMenu;
        m_trainMenu = nullptr;

        m_choiceMenu.setCursorVisible(true);
      }
    }
    else
    {
      escape();
    }
  }
}

void SkillTrainer::escape()
{
  close();
}

PlayerCharacter* SkillTrainer::getCurrentCharacter()
{
  return get_player()->getCharacter(m_choiceMenu.getCurrentMenuChoice());
}
