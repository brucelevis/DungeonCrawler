#include "Frame.h"
#include "Config.h"
#include "draw_text.h"
#include "Monster.h"
#include "Menu_BattleMonsterMenu.h"

BattleMonsterMenu::BattleMonsterMenu(const Callback& callback, const std::vector<Character*>& monsters)
 : m_monsters(monsters),
   m_callback(callback),
   m_index(0)
{
  setCursorVisible(false);
}

bool BattleMonsterMenu::handleInput(sf::Keyboard::Key key)
{
  int prev = m_index;

  switch (key)
  {
  case sf::Keyboard::Left:
    m_index--;

    while (getCurrentMonster()->getStatus() == "Dead")
    {
      m_index--;
      if (m_index < 0)
      {
        m_index = prev;
      }
    }

    break;
  case sf::Keyboard::Right:
    m_index++;

    while (getCurrentMonster()->getStatus() == "Dead")
    {
      m_index++;
      if (m_index >= static_cast<int>(m_monsters.size()))
      {
        m_index = prev;
      }
    }
    break;
  case sf::Keyboard::Space:
  case sf::Keyboard::Return:
    if (m_callback)
    {
      m_callback(getCurrentMonster());
    }
    break;
  case sf::Keyboard::Escape:
    if (m_callback)
    {
      m_callback(nullptr);
    }
    break;
  default:
    break;
  }

  if (m_index < 0)
  {
    m_index = m_monsters.size() - 1;
  }
  else if (m_index >= static_cast<int>(m_monsters.size()))
  {
    m_index = 0;
  }

  return true;
}

void BattleMonsterMenu::draw(sf::RenderTarget& target)
{
  int diff = 12;

  while (true)
  {
    int totalWidth = 0;
    for (size_t i = 0; i < m_monsters.size(); i++)
    {
      totalWidth += m_monsters[i]->spriteWidth() + diff;
    }
    if (totalWidth >= config::GAME_RES_X)
    {
      diff--;
    }
    else
    {
      break;
    }
  }

  for (size_t i = 0; i < m_monsters.size(); i++)
  {
    const Character* monster = m_monsters[i];

    // If monster is fading out, still draw it during that time.
    if (monster->getStatus() == "Dead" && !monster->flash().isFading())
      continue;

    int posX = config::GAME_RES_X / 2;
    int posY = config::GAME_RES_Y / 2;

    // Center adjustment for evenly sized monster groups.
    if ((m_monsters.size() % 2) == 0)
    {
      posX += config::GAME_RES_X / 8;
    }

    posX -= monster->spriteWidth() / 2;
    posY -= monster->spriteHeight() - 16;// / 2;

    posX -= (m_monsters.size() / 2 - i) * (monster->spriteWidth() + diff);

    monster->draw(target, posX, posY);

    if (cursorVisible() && m_index == static_cast<int>(i))
    {
      sf::RectangleShape rect = make_select_rect(posX - 2, posY - 2, monster->spriteWidth() + 2, monster->spriteHeight() + 2);
      target.draw(rect);

      draw_frame(target, 0, 0, config::GAME_RES_X, 24);
      draw_text_bmp(target, 8, 8, "%s", get_monster_description(monster->getName()).c_str());

      // Fix the position of the monster name so it doesn't go outside the screen.
      int textPosX = posX + monster->spriteWidth() / 2 - 8*(monster->getName().size() / 2);
      if (textPosX < 0) textPosX = 0;
      while (textPosX + (int)monster->getName().size() * 8 > config::GAME_RES_X) textPosX--;

      draw_text_bmp(target,
          textPosX,
          posY + monster->spriteHeight() + 4,
          "%s", monster->getName().c_str());
    }
  }
}

Character* BattleMonsterMenu::getCurrentMonster()
{
  return m_monsters[m_index];
}

void BattleMonsterMenu::fixSelection()
{
  if (getCurrentMonster()->getStatus() == "Dead")
  {
    for (size_t i = 0; i < m_monsters.size(); i++)
    {
      if (m_monsters[i]->getStatus() != "Dead")
      {
        m_index = i;
        break;
      }
    }
  }
}

void BattleMonsterMenu::addMonster(Character* monster)
{
  m_monsters.push_back(monster);
}
