#include "Frame.h"
#include "draw_text.h"
#include "Utility.h"
#include "Menu_BattleActionMenu.h"

const int MAX_VISIBLE_ACTION_ENTRIES = 4;

BattleActionMenu::BattleActionMenu(const ConfirmCallback& confirmCallback, const EscapeCallback& escapeCallback, int x, int y)
  : m_x(x),
    m_y(y),
    m_presenter(MenuPresenter::STYLE_FRAME),
    m_confirmCallback(confirmCallback),
    m_escapeCallback(escapeCallback)
{
  m_presenter.addEntry("Attack");
  m_presenter.addEntry("Spell");
  m_presenter.addEntry("Item");
  m_presenter.addEntry("Guard");
  m_presenter.addEntry("Run");
  m_presenter.setMaxVisible(MAX_VISIBLE_ACTION_ENTRIES);
}

bool BattleActionMenu::handleInput(sf::Keyboard::Key key)
{
  if (!cursorVisible() || !isVisible())
  {
    return false;
  }

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
    if (m_confirmCallback)
    {
      m_confirmCallback(m_presenter.getSelectedOption().entryName);
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

  return true;
}

void BattleActionMenu::draw(sf::RenderTarget& target)
{
  const int x = m_x;
  const int y = m_y;

  m_presenter.draw(target, x, y, this);
}

void BattleActionMenu::addEntry(const std::string& option)
{
  m_presenter.addEntry(option);
}

void BattleActionMenu::init(PlayerCharacter* character)
{
  m_presenter.clear();

  for (auto it = character->getClass().battleActions.begin();
       it != character->getClass().battleActions.end();
       ++it)
  {
    addEntry(*it);
  }

  m_presenter.setMaxVisible(MAX_VISIBLE_ACTION_ENTRIES);
}

void BattleActionMenu::resetChoice()
{
  m_presenter.reset();
}

std::string BattleActionMenu::getCurrentMenuChoice() const
{
  return m_presenter.getSelectedOption().entryName;
}
