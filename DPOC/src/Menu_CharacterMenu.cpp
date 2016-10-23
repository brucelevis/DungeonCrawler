#include "Frame.h"
#include "Player.h"
#include "draw_text.h"
#include "MenuTextHelpers.h"
#include "StatusEffect.h"
#include "draw_text.h"

#include "Menu_CharacterMenu.h"

CharacterMenu::CharacterMenu(const Callback& callback, const EscapeCallback& escapeCallback, int x, int y)
  : m_x(x),
    m_y(y),
    m_spellToUse(nullptr),
    m_user(nullptr),
    m_target(nullptr),
    m_callback(callback),
    m_escapeCallback(escapeCallback)
{
  setCursorVisible(false);

  const std::vector<PlayerCharacter*>& party = get_player()->getParty();

  for (PlayerCharacter* character : party)
  {
    m_characters.push_back(character);
    m_presenter.addEntry(character->getName());
  }
}

void CharacterMenu::reset()
{
  m_presenter.reset();
}

bool CharacterMenu::handleInput(sf::Keyboard::Key key)
{
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
    if (m_callback)
    {
      m_callback(m_characters[m_presenter.getSelectedOption().entryIndex]);
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

void CharacterMenu::draw(sf::RenderTarget& target)
{
  const int width = 184;
  const int height = 240;

  draw_frame(target, m_x, m_y, width, height);

  for (int i = 0; i < static_cast<int>(m_characters.size()); i++)
  {
    PlayerCharacter* character = m_characters[i];

    int offX = m_x + 16;
    int offY = m_y + 8;

    character->draw(target, offX, offY + i * 48);

    draw_text_bmp_ex(target, offX + 40, offY + i * 48,
        get_status_effect(character->getStatus())->color,
        "%s (%s)", character->getName().c_str(), character->getStatus().c_str());

    draw_hp(target, character, offX + 40, offY + i * 48 + 12);
    draw_mp(target, character, offX + 40, offY + i * 48 + 24);

    if (cursorVisible() && i == m_presenter.getSelectedOption().entryIndex)
    {
      sf::RectangleShape rect = make_select_rect(offX - 2, offY + i * 48 - 2, 164, 36);
      target.draw(rect);
    }
  }
}

void CharacterMenu::setUser(PlayerCharacter* character)
{
  m_user = character;
}

void CharacterMenu::setTarget(PlayerCharacter* character)
{
  m_target = character;
}
