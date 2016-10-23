#include "Frame.h"
#include "Config.h"
#include "Player.h"
#include "Utility.h"
#include "GuiStack.h"
#include "Vocabulary.h"
#include "StatusEffect.h"

#include "Menu_BattleActionMenu.h"
#include "Menu_BattleStatusMenu.h"

BattleStatusMenu::BattleStatusMenu(const ConfirmCallback& confirmCallback, const EscapeCallback& escapeCallback, int x, int y)
  : m_x(x),
    m_y(y),
    m_currentActor(0),
    m_currenActorRectHidden(false),
    m_index(0),
    m_confirmCallback(confirmCallback),
    m_escapeCallback(escapeCallback)
{
  m_party = get_player()->getParty();
  resetActor();
}

bool BattleStatusMenu::handleInput(sf::Keyboard::Key key)
{
  switch (key)
  {
  case sf::Keyboard::Up:
    m_index--;
    break;
  case sf::Keyboard::Down:
    m_index++;
    break;
  case sf::Keyboard::Space:
  case sf::Keyboard::Return:
    if (m_confirmCallback)
    {
      m_confirmCallback(getCurrentSelectedActor());
    }
    break;
  case sf::Keyboard::Escape:
    if (m_escapeCallback)
    {
      m_escapeCallback();
    }
    break;
  default:
    break;
  }

  if (m_index < 0) m_index = 0;
  else if (m_index >= static_cast<int>(m_party.size())) m_index = m_party.size() - 1;

  return true;
}

void BattleStatusMenu::draw(sf::RenderTarget& target)
{
  draw_frame(target, m_x, m_y, getWidth(), getHeight());

  for (size_t i = 0; i < m_party.size(); i++)
  {
    PlayerCharacter* character = m_party[i];
    std::string name = character->getName();

    int offY = m_y + 8 + i * ENTRY_OFFSET;

    float hpPercent = (float)character->getAttribute(terms::hp).current / (float)character->getAttribute(terms::hp).max;

    draw_text_bmp(target, m_x + 8,  offY, "%s", limit_string(name, 5).c_str());
    draw_text_bmp_ex(target, m_x + 56, offY,
        get_status_effect(character->getStatus())->color,
        "%s", limit_string(character->getStatus(), 4).c_str());
    draw_text_bmp_ex(target, m_x + 100, offY,
        hpPercent > 0.2 ? sf::Color::White : sf::Color::Red,
        "%d", character->getAttribute(terms::hp).current);
    draw_text_bmp(target, m_x + 136, offY, "%d", character->getAttribute(terms::mp).current);

    if (i == m_currentActor && !m_currenActorRectHidden)
    {
      sf::RectangleShape rect = make_select_rect(m_x + 6, offY - 1, getWidth() - 12, 11, sf::Color::White);
      target.draw(rect);
    }

    if (cursorVisible())
    {
      if (i == m_index)
      {
        sf::RectangleShape rect = make_select_rect(m_x + 6, offY - 1, getWidth() - 12, 11, sf::Color::Red);
        target.draw(rect);
      }
    }
  }
}

int BattleStatusMenu::getWidth() const
{
  return config::GAME_RES_X - 80;
}

int BattleStatusMenu::getHeight() const
{
  return 2 * 8 + 4 * 12;
}

bool BattleStatusMenu::prevActor()
{
  int tmpIndex = m_currentActor;

  m_currentActor--;

  if (m_currentActor >= 0)
  {
    while (getCurrentActor()->incapacitated())
    {
      m_currentActor--;
      if (m_currentActor < 0)
      {
        m_currentActor = tmpIndex;

        refreshActionMenu();
        return false;
      }
    }
  }

  if (m_currentActor < 0)
  {
    m_currentActor = 0;

    refreshActionMenu();
    return false;
  }

  refreshActionMenu();
  return true;
}

bool BattleStatusMenu::nextActor()
{
  int tmpIndex = m_currentActor;

  m_currentActor++;

  if (m_currentActor < getPartySize())
  {
    while (getCurrentActor()->incapacitated())
    {
      m_currentActor++;
      if (m_currentActor >= getPartySize())
      {
        m_currentActor = tmpIndex;

        refreshActionMenu();
        return false;
      }
    }
  }

  if (m_currentActor >= getPartySize())
  {
    m_currentActor = getPartySize() - 1;

    refreshActionMenu();
    return false;
  }

  refreshActionMenu();
  return true;
}

PlayerCharacter* BattleStatusMenu::getCurrentActor()
{
  return get_player()->getParty().at(m_currentActor);
}

PlayerCharacter* BattleStatusMenu::getCurrentSelectedActor()
{
  return get_player()->getParty().at(m_index);
}

void BattleStatusMenu::resetActor()
{
  m_currentActor = 0;

  while (getCurrentActor()->incapacitated())
  {
    m_currentActor++;
    if (m_currentActor >= static_cast<int>(m_party.size()))
    {
      m_currentActor = m_party.size() - 1;
      break;
    }
  }

  refreshActionMenu();
}

void BattleStatusMenu::refreshActionMenu()
{
  if (auto actionMenu = getGuiStack()->findWidget<BattleActionMenu>())
  {
    actionMenu->init(getCurrentActor());
  }
}

int BattleStatusMenu::getPartySize() const
{
  return static_cast<int>(m_party.size());
}

void BattleStatusMenu::resetChoice()
{
  m_index = 0;
}
