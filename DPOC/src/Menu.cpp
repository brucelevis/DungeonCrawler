#include "Persistent.h"
#include "Config.h"
#include "logger.h"
#include "Cache.h"
#include "Frame.h"
#include "draw_text.h"
#include "Utility.h"
#include "Sound.h"

#include "StatusEffect.h"
#include "Attack.h"
#include "Player.h"
#include "Character.h"
#include "PlayerCharacter.h"
#include "Game.h"
#include "Item.h"
#include "Spell.h"
#include "Monster.h"
#include "Battle.h"

#include "Menu.h"

static const int ENTRY_OFFSET = 12;

static std::string get_longest_menu_choice(const std::vector<std::string>& menuChoices)
{
  std::string longest;

  for (auto it = menuChoices.begin(); it != menuChoices.end(); ++it)
  {
    if (it->size() > longest.size())
      longest = *it;
  }

  return longest;
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

Menu::Menu()
 : m_arrowTexture(cache::loadTexture("Resources/Arrow.png")),
   m_visible(false),
   m_currentMenuChoice(0),
   m_maxVisible(-1),
   m_scroll(0),
   m_cursorVisible(true)
{

}

Menu::~Menu()
{
  cache::releaseTexture("Resources/Arrow.png");
}

void Menu::moveArrow(Direction dir)
{
  if (dir == DIR_UP)
  {
    m_currentMenuChoice--;
  }
  else if (dir == DIR_DOWN)
  {
    m_currentMenuChoice++;
  }

  fixScroll(dir);
}

void Menu::fixScroll(Direction dir)
{
  if (dir == DIR_UP)
  {
    if (m_currentMenuChoice < 0)
    {
      m_currentMenuChoice = 0;
    }

    if (m_maxVisible != -1)
    {
      if (m_currentMenuChoice < m_scroll)
      {
        m_scroll--;
      }
    }
  }
  else if (dir == DIR_DOWN)
  {
    if (m_currentMenuChoice >= (int)m_menuChoices.size())
    {
      m_currentMenuChoice = m_menuChoices.size() - 1;
      if (m_currentMenuChoice < 0)
        m_currentMenuChoice = 0;
    }

    if (m_maxVisible != -1)
    {
      if (m_currentMenuChoice >= m_maxVisible + m_scroll)
      {
        m_scroll++;
      }
    }
  }
}

int Menu::getWidth() const
{
  return (4 + get_longest_menu_choice(m_menuChoices).size()) * 8;
}

int Menu::getHeight() const
{
  int end = m_maxVisible == -1 ? m_menuChoices.size() : m_maxVisible;
  return 2 * 8 + end * 12;
}

void Menu::draw(sf::RenderTarget& target, int x, int y)
{
  int start = m_maxVisible == -1 ? 0 : m_scroll;
  int end = m_maxVisible == -1 ? m_menuChoices.size() : (m_maxVisible + m_scroll);

  int w = getWidth();
  int h = getHeight();

  draw_frame(target, x, y, w, h);

  int i = 0;
  for (int index = start; index < end; index++, i++)
  {
    if (index < (int)m_menuChoices.size())
    {
      draw_text_bmp(target, x + 16, y + 8 + i * ENTRY_OFFSET, "%s", m_menuChoices[index].c_str());
    }

    if (m_currentMenuChoice == index && cursorVisible())
    {
      drawSelectArrow(target, x + 8, y + 8 + i * ENTRY_OFFSET);
    }
  }

  if (m_scroll > 0)
  {
    sf::Sprite sprite;
    sprite.setTexture(*m_arrowTexture);
    sprite.setPosition(x + w - 12, y + 4);
    sprite.setTextureRect(sf::IntRect(8, 0, 8, 8));
    target.draw(sprite);
  }

  if (end < (int)m_menuChoices.size())
  {
    sf::Sprite sprite;
    sprite.setTexture(*m_arrowTexture);
    sprite.setPosition(x + w - 12, y + h - 12);
    sprite.setTextureRect(sf::IntRect(16, 0, 8, 8));
    target.draw(sprite);
  }

}

void Menu::drawSelectArrow(sf::RenderTarget& target, int x, int y)
{
  sf::Sprite sprite;
  sprite.setTexture(*m_arrowTexture);
  sprite.setTextureRect(sf::IntRect(0, 0, 8, 8));
  sprite.setPosition(x, y);
  target.draw(sprite);
}

///////////////////////////////////////////////////////////////////////////////

void ChoiceMenu::handleConfirm()
{
  Persistent<int>::instance().set("sys:choice", getCurrentChoiceIndex());
}

///////////////////////////////////////////////////////////////////////////////

MainMenu::MainMenu()
 : m_itemMenu(0),
   m_spellMenu(0),
   m_characterMenu(new CharacterMenu),
   m_equipMenu(0)
{
  addEntry("Item");
  addEntry("Spell");
  addEntry("Equip");
  addEntry("Status");
  addEntry("Save");
  addEntry("Close");

  m_stateStack.push(STATE_MAIN_MENU);
}

MainMenu::~MainMenu()
{
  delete m_itemMenu;
  delete m_spellMenu;
  delete m_characterMenu;
  delete m_equipMenu;
}

void MainMenu::handleConfirm()
{
  State currentState = m_stateStack.top();

  if (currentState == STATE_MAIN_MENU)
  {
    if (getCurrentMenuChoice() == "Close")
    {
      setVisible(false);
    }
    else if (getCurrentMenuChoice() == "Item")
    {
      openItemMenu();
    }
    else if (getCurrentMenuChoice() == "Spell")
    {
      m_characterMenu->resetChoice();
      openCharacterMenu();
    }
    else if (getCurrentMenuChoice() == "Status")
    {
      m_characterMenu->resetChoice();
      openCharacterMenu();
    }
    else if (getCurrentMenuChoice() == "Equip")
    {
      m_characterMenu->resetChoice();
      openCharacterMenu();
    }
  }
  else if (currentState == STATE_CHARACTER_MENU)
  {
    if (getCurrentMenuChoice() == "Spell")
    {
      if (m_characterMenu->getSpellToUse())
      {
        m_characterMenu->setTargetToCurrentChoice();

        play_sound(config::SOUND_USE_ITEM);

        cast_spell(m_characterMenu->getSpellToUse(),
            m_characterMenu->getUser(),
            m_characterMenu->getTarget());

        // If we can't cast the selected spell, clsoe the char menu.
        if (!can_cast_spell(m_characterMenu->getSpellToUse(), m_characterMenu->getUser()))
        {
          closeCharacterMenu();
        }
      }
      else
      {
        m_stateStack.pop();
        m_characterMenu->setUserToCurrentChoice();
        openSpellMenu(m_characterMenu->getCurrentMenuChoice());
      }
    }
    else if (getCurrentMenuChoice() == "Item")
    {
      if (m_characterMenu->getItemToUse().size() > 0 && get_player()->getItem(m_characterMenu->getItemToUse()))
      {
        m_characterMenu->setUserToCurrentChoice();
        m_characterMenu->setTargetToCurrentChoice();

        play_sound(config::SOUND_USE_ITEM);

        Item* item = get_player()->getItem(m_characterMenu->getItemToUse());
        use_item(item, m_characterMenu->getUser(), m_characterMenu->getTarget());

        get_player()->removeItemFromInventory(m_characterMenu->getItemToUse(), 1);

        m_itemMenu->refresh();

        // Close if no more items.
//        if (get_player()->getItem(m_characterMenu->getItemToUse()) == 0)
//        {
//          closeCharacterMenu();
//        }
      }
    }
    else if (getCurrentMenuChoice() == "Status")
    {
      m_stateStack.push(STATE_STATUS_MENU);
    }
    else if (getCurrentMenuChoice() == "Equip")
    {
      m_characterMenu->setUserToCurrentChoice();
      openEquipMenu();
    }
  }
  else if (currentState == STATE_SPELL_MENU)
  {
    const Spell* spell = m_spellMenu->getSelectedSpell();
    if (spell && !spell->battleOnly && can_cast_spell(spell, m_characterMenu->getUser()))
    {
      m_characterMenu->setSpellToUse(spell);
      openCharacterMenu();
    }
    else
    {
      play_sound(config::SOUND_CANCEL);
    }
  }
  else if (currentState == STATE_ITEM_MENU)
  {
    std::string itemName = m_itemMenu->getSelectedItemName();
    Item* item = get_player()->getItem(itemName);
    if (item && item->type == ITEM_USE && item->target == TARGET_SINGLE_ALLY)
    {
      m_characterMenu->setItemToUse(itemName);
      openCharacterMenu();
    }
    else
    {
      play_sound(config::SOUND_CANCEL);
    }
  }
  else if (currentState == STATE_EQUIP_MENU)
  {
    m_equipMenu->handleConfirm();
  }
}

void MainMenu::handleEscape()
{
  State currentState = m_stateStack.top();

  if (currentState != STATE_MAIN_MENU)
  {
    if (currentState == STATE_ITEM_MENU)
    {
      closeItemMenu();
    }
    else if (currentState == STATE_SPELL_MENU)
    {
      closeSpellMenu();
    }
    else if (currentState == STATE_CHARACTER_MENU)
    {
      closeCharacterMenu();
    }
    else if (currentState == STATE_EQUIP_MENU)
    {
      m_equipMenu->handleEscape();
      if (!m_equipMenu->isVisible())
      {
        closeEquipMenu();
      }
    }
    else if (currentState == STATE_STATUS_MENU)
    {
      m_stateStack.pop();
    }
  }
  else
  {
    Menu::handleEscape();
  }
}

void MainMenu::moveArrow(Direction dir)
{
  State currentState = m_stateStack.top();

  if (currentState == STATE_ITEM_MENU)
  {
    m_itemMenu->moveArrow(dir);
  }
  else if (currentState == STATE_CHARACTER_MENU || currentState == STATE_STATUS_MENU)
  {
    m_characterMenu->moveArrow(dir);
  }
  else if (currentState == STATE_SPELL_MENU)
  {
    m_spellMenu->moveArrow(dir);
  }
  else if (currentState == STATE_MAIN_MENU)
  {
    Menu::moveArrow(dir);
  }
  else if (currentState == STATE_EQUIP_MENU)
  {
    m_equipMenu->moveArrow(dir);
  }
}

void MainMenu::openItemMenu()
{
  m_itemMenu = new ItemMenu;
  m_itemMenu->setVisible(true);

  m_stateStack.push(STATE_ITEM_MENU);
}

void MainMenu::closeItemMenu()
{
  delete m_itemMenu;
  m_itemMenu = 0;

  m_stateStack.pop();
}

void MainMenu::openSpellMenu(const std::string& characterName)
{
  m_spellMenu = new SpellMenu(characterName);
  m_spellMenu->setVisible(true);

  m_stateStack.push(STATE_SPELL_MENU);
}

void MainMenu::closeSpellMenu()
{
  delete m_spellMenu;
  m_spellMenu = 0;

  m_stateStack.pop();
}

void MainMenu::openCharacterMenu()
{
  m_characterMenu->setCursorVisible(true);
  m_stateStack.push(STATE_CHARACTER_MENU);
}

void MainMenu::closeCharacterMenu()
{
  m_characterMenu->setCursorVisible(false);
  m_characterMenu->setSpellToUse(0);
  m_characterMenu->setItemToUse("");

  m_stateStack.pop();
}

void MainMenu::openEquipMenu()
{
  m_equipMenu = new EquipMenu(m_characterMenu->getUser());
  m_equipMenu->setVisible(true);

  m_stateStack.push(STATE_EQUIP_MENU);
}

void MainMenu::closeEquipMenu()
{
  delete m_equipMenu;
  m_equipMenu = 0;

  m_stateStack.pop();
}

void MainMenu::open()
{
  setVisible(true);
  resetChoice();

  m_characterMenu->refresh();
  m_characterMenu->setVisible(true);
}

void MainMenu::draw(sf::RenderTarget& target, int x, int y)
{
  State currentState = m_stateStack.top();

  if (currentState == STATE_EQUIP_MENU)
  {
    m_equipMenu->draw(target, 0, 0);
  }
  else
  {
    draw_frame(target, x, y, config::GAME_RES_X, config::GAME_RES_Y);

    draw_frame(target, x, y, 80, 96);
    draw_frame(target, x, y + 208, 80, 32);

    draw_text_bmp(target, x + 8, y + 13*16+7, "GP");
    draw_text_bmp(target, x + 8, y + 13*16+19, "%d", get_player()->getGold());

    for (int i = 0; i < getNumberOfChoice(); i++)
    {
      draw_text_bmp(target, x + 18, y + 10 + i * 14, "%s", getChoice(i).c_str());
      if (i == getCurrentChoiceIndex())
      {
        drawSelectArrow(target, x + 8, y + 10 + i * 14);
      }
    }

    if (m_characterMenu && m_characterMenu->isVisible())
    {
      m_characterMenu->draw(target, x, y);
    }

    if (currentState == STATE_STATUS_MENU)
    {
      drawStatus(target, x + 24, y + 24);
    }
    else if (currentState == STATE_ITEM_MENU || !m_characterMenu->getItemToUse().empty())
    {
      m_itemMenu->draw(target, x + 16, y + 16);

      if (m_characterMenu->getItemToUse().size() > 0)
      {
        m_characterMenu->draw(target, x, y);
      }
    }
    else if (currentState == STATE_SPELL_MENU || m_characterMenu->getSpellToUse())
    {
      m_spellMenu->draw(target, x + 16, y + 16);

      if (m_characterMenu->getSpellToUse())
      {
        m_characterMenu->draw(target, x, y);
      }
    }
  }
}

void MainMenu::drawStatus(sf::RenderTarget& target, int x, int y)
{
  PlayerCharacter* character = Game::instance().getPlayer()->getCharacter(m_characterMenu->getCurrentMenuChoice());

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
  draw_text_bmp(target, x, y + 12, "Power:    %d", character->computeCurrentAttribute("power"));
  draw_text_bmp(target, x, y + 24, "Defense:  %d", character->computeCurrentAttribute("defense"));
  draw_text_bmp(target, x, y + 36, "Magic:    %d", character->computeCurrentAttribute("magic"));
  draw_text_bmp(target, x, y + 48, "Mag.Def:  %d", character->computeCurrentAttribute("mag.def"));
  draw_text_bmp(target, x, y + 60, "Speed:    %d", character->computeCurrentAttribute("speed"));

  for (size_t i = 0; i < PlayerCharacter::equipNames.size(); i++)
  {
    Item* item = character->getEquipment(PlayerCharacter::equipNames[i]);
    draw_text_bmp(target, x, y + 84 + 12 * i, "%s: %s", PlayerCharacter::equipNames[i].c_str(), item ? item->name.c_str(): "");
  }
}

///////////////////////////////////////////////////////////////////////////////

ItemMenu::ItemMenu()
 : m_width(14*16),
   m_height(12*16)
{
  refresh();
}

ItemMenu::ItemMenu(int width, int height)
 : m_width(width),
   m_height(height)
{
  refresh();
}

void ItemMenu::handleConfirm()
{

}

void ItemMenu::refresh()
{
  clear();

  const std::vector<Item>& items = Game::instance().getPlayer()->getInventory();

  for (auto it = items.begin(); it != items.end(); ++it)
  {
    std::string stack = toString(it->stackSize);
    std::string name = it->name;

    // Add some padding.
    if (stack.size() == 1)
      stack += " ";

    addEntry(stack + " " + name);

    m_items.push_back(&(*it));
  }

  if (getCurrentChoiceIndex() >= getNumberOfChoice())
  {
    resetChoice();
  }

  int visible = m_height / 8 - 2;

  setMaxVisible(visible);
}

void ItemMenu::draw(sf::RenderTarget& target, int x, int y)
{
  draw_frame(target, x, y, getWidth(), 3*16);

  Menu::draw(target, x, y + 24);

  if (cursorVisible() && hasItem(getSelectedItemName()))
  {
    draw_text_bmp(target, x + 8, y + 8, "%s", getItem(getSelectedItemName())->description.c_str());
  }
}

bool ItemMenu::hasItem(const std::string& name) const
{
  for (auto it = m_items.begin(); it != m_items.end(); ++it)
  {
    if ((*it)->name == name)
      return true;
  }
  return false;
}

const Item* ItemMenu::getItem(const std::string& name) const
{
  for (auto it = m_items.begin(); it != m_items.end(); ++it)
  {
    if ((*it)->name == name)
    {
      return *it;
    }
  }
  return 0;
}

int ItemMenu::getWidth() const
{
  return m_width;
}

int ItemMenu::getHeight() const
{
  return m_height;
}

std::string ItemMenu::getSelectedItemName() const
{
  return get_string_after_first_space(getCurrentMenuChoice());
}

///////////////////////////////////////////////////////////////////////////////

EquipItemMenu::EquipItemMenu(int width, int height)
 : ItemMenu(width, height)
{

}

void EquipItemMenu::refresh(const std::string& equipmentType)
{
  clear();

  const std::vector<Item>& items = get_player()->getInventory();

  addEntry("* Remove *");

  for (auto it = items.begin(); it != items.end(); ++it)
  {
    if (equip_type_string(it->type) == equipmentType)
    {
      std::string stack = toString(it->stackSize);
      std::string name = it->name;

      // Add some padding.
      if (stack.size() == 1)
        stack += " ";

      addEntry(stack + " " + name);

      m_items.push_back(&(*it));
    }
  }

  if (getCurrentChoiceIndex() >= getNumberOfChoice())
  {
    resetChoice();
  }

  int visible = m_height / 8 - 2;

  setMaxVisible(visible);
}

///////////////////////////////////////////////////////////////////////////////

SpellMenu::SpellMenu(const std::string& characterName)
{
  const std::vector<std::string>& spells = Game::instance().getPlayer()->getCharacter(characterName)->getSpells();

  for (auto it = spells.begin(); it != spells.end(); ++it)
  {
    const Spell* spell = get_spell(*it);

    if (spell)
    {
      std::string costString = toString(spell->mpCost);

      // Add some padding
      if (costString.size() == 1)
      {
        costString += " ";
      }

      addEntry(costString + " " + spell->name);
    }
  }

  setMaxVisible(10);
}

void SpellMenu::handleConfirm()
{

}

const Spell* SpellMenu::getSelectedSpell() const
{
  if (getNumberOfChoice() == 0)
    return 0;

  std::string spellName = get_string_after_first_space(getCurrentMenuChoice());

  return get_spell(spellName);
}

void SpellMenu::draw(sf::RenderTarget& target, int x, int y)
{
  draw_frame(target, x, y, getWidth(), 3*16);

  Menu::draw(target, x, y + 24);

  draw_text_bmp(target, x + 8, y + 8, "%s", getSelectedSpell() ? getSelectedSpell()->description.c_str() : "");
}

int SpellMenu::getWidth() const
{
  return 14*16;
}

int SpellMenu::getHeight() const
{
  return 12*16;
}

///////////////////////////////////////////////////////////////////////////////

CharacterMenu::CharacterMenu()
 : m_spellToUse(0),
   m_user(0),
   m_target(0)
{
  setCursorVisible(false);
}

void CharacterMenu::handleConfirm()
{

}

int CharacterMenu::getWidth() const
{
  return Menu::getWidth() + 32;
}

int CharacterMenu::getHeight() const
{
  return (4 + 32) * getNumberOfChoice();
}

void CharacterMenu::refresh()
{
  clear();

  const std::vector<PlayerCharacter*>& party = get_player()->getParty();

  for (auto it = party.begin(); it != party.end(); ++it)
  {
    addEntry((*it)->getName());
  }
}

void CharacterMenu::draw(sf::RenderTarget& target, int x, int y)
{
  draw_frame(target, x + 72, y, 184, 240);

  for (int i = 0; i < getNumberOfChoice(); i++)
  {
    PlayerCharacter* character = get_player()->getCharacter(getChoice(i));

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

void CharacterMenu::setUserToCurrentChoice()
{
  m_user = Game::instance().getPlayer()->getCharacter(getCurrentMenuChoice());
}

void CharacterMenu::setTargetToCurrentChoice()
{
  m_target = Game::instance().getPlayer()->getCharacter(getCurrentMenuChoice());
}

///////////////////////////////////////////////////////////////////////////////

EquipMenu::EquipMenu(PlayerCharacter* character)
 : m_character(character),
   m_itemMenu(new EquipItemMenu(16*16, 8*16)),
   m_state(STATE_SELECT_EQUIPMENT_TYPE)
{
  for (auto it = PlayerCharacter::equipNames.begin(); it != PlayerCharacter::equipNames.end(); ++it)
  {
    addEntry(*it);
  }

  m_itemMenu->setCursorVisible(false);
  m_itemMenu->refresh(getCurrentMenuChoice());
}

EquipMenu::~EquipMenu()
{
  delete m_itemMenu;
}

void EquipMenu::handleConfirm()
{
  if (m_state == STATE_SELECT_EQUIPMENT_TYPE)
  {
    m_state = STATE_EQUIP_ITEM;
    m_itemMenu->setCursorVisible(true);
    m_itemMenu->resetChoice();
  }
  else if (m_state == STATE_EQUIP_ITEM)
  {
    m_itemMenu->handleConfirm();

    if (m_itemMenu->validChoice())
    {
      doEquip();
    }
    else
    {
      doUnEquip();
    }
  }
}

void EquipMenu::doEquip()
{
  std::string currentItemName = m_itemMenu->getSelectedItemName();
  Item* currentItem = get_player()->getItem(currentItemName);
  Item* currentEquip = m_character->getEquipment(getCurrentMenuChoice());

  if (currentItem && getCurrentMenuChoice() == equip_type_string(currentItem->type) && m_character->canEquip(currentItemName))
  {
    if (currentEquip)
    {
      get_player()->addItemToInventory(currentEquip->name, 1);
    }

    m_character->equip(getCurrentMenuChoice(), currentItem->name);
    get_player()->removeItemFromInventory(currentItem->name, 1);

    m_itemMenu->refresh(getCurrentMenuChoice());
    m_itemMenu->setCursorVisible(false);

    m_state = STATE_SELECT_EQUIPMENT_TYPE;

    play_sound(config::SOUND_EQUIP);
  }
  else
  {
    play_sound(config::SOUND_CANCEL);
  }
}

void EquipMenu::doUnEquip()
{
  Item* currentEquip = m_character->getEquipment(getCurrentMenuChoice());

  if (currentEquip)
  {
    get_player()->addItemToInventory(currentEquip->name, 1);
    m_character->equip(getCurrentMenuChoice(), "");

    m_itemMenu->refresh(getCurrentMenuChoice());
    m_itemMenu->setCursorVisible(false);

    m_state = STATE_SELECT_EQUIPMENT_TYPE;

    play_sound(config::SOUND_EQUIP);
  }
  else
  {
    play_sound(config::SOUND_CANCEL);
  }
}

void EquipMenu::handleEscape()
{
  if (m_state == STATE_SELECT_EQUIPMENT_TYPE)
  {
    Menu::handleEscape();
  }
  else if (m_state == STATE_EQUIP_ITEM)
  {
    m_state = STATE_SELECT_EQUIPMENT_TYPE;
    m_itemMenu->setCursorVisible(false);
  }
}

void EquipMenu::moveArrow(Direction dir)
{
  if (m_state == STATE_SELECT_EQUIPMENT_TYPE)
  {
    Menu::moveArrow(dir);
    m_itemMenu->refresh(getCurrentMenuChoice());
  }
  else
  {
    m_itemMenu->moveArrow(dir);
  }
}

void EquipMenu::draw(sf::RenderTarget& target, int x, int y)
{
  draw_frame(target, x, y, 8*16, 7*16);
  draw_frame(target, x + 8*16, y, 8*16, 7*16);

  // Top.
  draw_frame(target, x, y, 16 * 16, 2 * 16);
  m_character->draw(target, x, y);
  draw_text_bmp(target, x + 36, y + 8, "%s", m_character->getName().c_str());

  m_itemMenu->draw(target, 0, 7 * 16);

  int offX = x + 8;
  int offY = y + 38;

  drawDeltas(target, offX, offY);

  offX = x + 136;

  for (int i = 0; i < getNumberOfChoice(); i++)
  {
    Item* equipment = m_character->getEquipment(getChoice(i));

    std::string typeShortName = get_equip_short_name(getChoice(i));
    std::string itemShortName = equipment ?
        limit_string(equipment->name, 8) :
        "";

    draw_text_bmp(target, offX, offY + 12 * i, "%s: %s", typeShortName.c_str(), itemShortName.c_str());
  }

  sf::RectangleShape rect = make_select_rect(offX - 1, offY - 1 + getCurrentChoiceIndex() * 12, 114, 10);
  target.draw(rect);
}

void EquipMenu::drawDeltas(sf::RenderTarget& target, int x, int y)
{
  Item* currentEquip = m_character->getEquipment(getCurrentMenuChoice());

  if (m_itemMenu->validChoice())
  {
    m_character->equip(getCurrentMenuChoice(), m_itemMenu->getSelectedItemName());
  }
  else
  {
    m_character->equip(getCurrentMenuChoice(), "");
  }

  int newStr = m_character->computeCurrentAttribute("strength");
  int newPow = m_character->computeCurrentAttribute("power");
  int newDef = m_character->computeCurrentAttribute("defense");
  int newMag = m_character->computeCurrentAttribute("magic");
  int newMdf = m_character->computeCurrentAttribute("mag.def");
  int newSpd = m_character->computeCurrentAttribute("speed");

  m_character->equip(getCurrentMenuChoice(), currentEquip ? currentEquip->name : "");

  draw_text_bmp(target, x, y,      "Str: %d (%d)", m_character->computeCurrentAttribute("strength"), newStr);
  draw_text_bmp(target, x, y + 12, "Pow: %d (%d)", m_character->computeCurrentAttribute("power"), newPow);
  draw_text_bmp(target, x, y + 24, "Def: %d (%d)", m_character->computeCurrentAttribute("defense"), newDef);
  draw_text_bmp(target, x, y + 36, "Mag: %d (%d)", m_character->computeCurrentAttribute("magic"), newMag);
  draw_text_bmp(target, x, y + 48, "Mdf: %d (%d)", m_character->computeCurrentAttribute("mag.def"), newMdf);
  draw_text_bmp(target, x, y + 60, "Spd: %d (%d)", m_character->computeCurrentAttribute("speed"), newSpd);
}

///////////////////////////////////////////////////////////////////////////////

BattleMenu::BattleMenu(Battle* battle, const std::vector<Character*>& monsters)
 : m_actionMenu(new BattleActionMenu),
   m_statusMenu(new BattleStatusMenu),
   m_monsterMenu(new BattleMonsterMenu(monsters)),
   m_spellMenu(0),
   m_itemMenu(0),
   m_battle(battle),
   m_actionMenuHidden(false)
{
  setCursorVisible(true);
  setVisible(true);
  m_stateStack.push(STATE_SELECT_ACTION);
}

BattleMenu::~BattleMenu()
{
  delete m_actionMenu;
  delete m_statusMenu;
  delete m_monsterMenu;

  delete m_spellMenu;
  delete m_itemMenu;
}

void BattleMenu::handleConfirm()
{
  State currentState = m_stateStack.top();

  if (currentState == STATE_SELECT_ACTION)
  {
    std::string action = m_actionMenu->getCurrentMenuChoice();

    if (action == "Attack")
    {
      selectMonster();
    }
    else if (action == "Spell")
    {
      m_spellMenu = new SpellMenu(m_statusMenu->getCurrentActor()->getName());
      m_spellMenu->setVisible(true);
      m_stateStack.push(STATE_SELECT_SPELL);
    }
    else if (action == "Item")
    {
      m_itemMenu = new ItemMenu;
      m_itemMenu->setVisible(true);
      m_stateStack.push(STATE_SELECT_ITEM);
    }
    else if (action == "Guard")
    {
      prepareAction();

      nextActor();
    }
    else if (action == "Run")
    {
      prepareAction();

      nextActor();
    }
  }
  else if (currentState == STATE_SELECT_MONSTER)
  {
    prepareAction();

    nextActor();
  }
  else if (currentState == STATE_SELECT_CHARACTER)
  {
    // TODO: Spells/items that can target dead
    if (m_statusMenu->getCurrentSelectedActor()->getStatus() == "Dead")
    {
      play_sound(config::SOUND_CANCEL);
    }
    else
    {
      prepareAction();

      nextActor();
    }
  }
  else if (currentState == STATE_SELECT_SPELL)
  {
    const Spell* spell = m_spellMenu->getSelectedSpell();

    if (spell->target == TARGET_NONE || spell->mpCost > m_statusMenu->getCurrentActor()->getAttribute("mp").current)
    {
      play_sound(config::SOUND_CANCEL);
    }
    else if (spell->target == TARGET_SINGLE_ENEMY)
    {
      m_spellMenu->setVisible(false);
      selectMonster();
    }
    else if (spell->target == TARGET_SINGLE_ALLY)
    {
      m_spellMenu->setVisible(false);
      selectCharacter();
    }
    else if (spell->target == TARGET_ALL_ENEMY ||
             spell->target == TARGET_ALL_ALLY ||
             spell->target == TARGET_SELF)
    {
      prepareAction();

      nextActor();
    }
  }
  else if (currentState == STATE_SELECT_ITEM)
  {
    const Item* item = get_player()->getItem(m_itemMenu->getSelectedItemName());

    if (item->target == TARGET_NONE || item->type != ITEM_USE)
    {
      play_sound(config::SOUND_CANCEL);
    }
    else if (item->target == TARGET_SINGLE_ENEMY)
    {
      m_itemMenu->setVisible(false);
      selectMonster();
    }
    else if (item->target == TARGET_SINGLE_ALLY)
    {
      m_itemMenu->setVisible(false);
      selectCharacter();
    }
    else if (item->target == TARGET_ALL_ENEMY ||
             item->target == TARGET_ALL_ALLY ||
             item->target == TARGET_SELF)
    {
      prepareAction();

      nextActor();
    }
  }
}

void BattleMenu::nextActor()
{
  closeSpellMenu();
  closeItemMenu();

  m_monsterMenu->setCursorVisible(false);
  m_statusMenu->setCursorVisible(false);

  while (m_stateStack.top() != STATE_SELECT_ACTION)
  {
    m_stateStack.pop();
  }

  if (!m_statusMenu->nextActor())
  {
    // If no more actors, we are done.
    m_battle->doneSelectingActions();
  }

  m_actionMenu->resetChoice();
}

void BattleMenu::prepareAction()
{
  std::string action = m_actionMenu->getCurrentMenuChoice();

  Battle::Action battleAction;
  battleAction.actionName = action;

  if (action == "Attack")
  {
    battleAction.target = m_monsterMenu->getCurrentMonster();
  }
  else if (action == "Spell")
  {
    const Spell* spell = m_spellMenu->getSelectedSpell();

    battleAction.objectName = spell->name;
    battleAction.target = getTarget(spell->target);
  }
  else if (action == "Item")
  {
    const Item* item = get_player()->getItem(m_itemMenu->getSelectedItemName());

    battleAction.objectName = item->name;
    battleAction.target = getTarget(item->target);
  }
  else if (action == "Guard")
  {
    battleAction.target = 0;
  }
  else if (action == "Run")
  {
    battleAction.target = 0;
  }

  m_battle->setAction(m_statusMenu->getCurrentActor(), battleAction);
}

void BattleMenu::selectMonster()
{
  m_monsterMenu->setCursorVisible(true);
  m_monsterMenu->fixSelection();
  m_stateStack.push(STATE_SELECT_MONSTER);
}

void BattleMenu::selectCharacter()
{
  m_statusMenu->setCursorVisible(true);
  m_statusMenu->resetChoice();
  m_stateStack.push(STATE_SELECT_CHARACTER);
}

void BattleMenu::handleEscape()
{
  State currentState = m_stateStack.top();

  if (currentState == STATE_SELECT_MONSTER)
  {
    m_monsterMenu->setCursorVisible(false);
  }
  else if (currentState == STATE_SELECT_ITEM)
  {
    closeItemMenu();
  }
  else if (currentState == STATE_SELECT_SPELL)
  {
    closeSpellMenu();
  }
  else if (currentState == STATE_SELECT_CHARACTER)
  {
    m_statusMenu->setCursorVisible(false);
  }

  if (m_stateStack.size() > 1)
  {
    m_stateStack.pop();

    currentState = m_stateStack.top();
    if (currentState == STATE_SELECT_SPELL)
    {
      m_spellMenu->setVisible(true);
    }
    else if (currentState == STATE_SELECT_ITEM)
    {
      m_itemMenu->setVisible(true);
    }
  }
  else
  {
    // Redo actions for previous actor.
    m_statusMenu->prevActor();
  }
}

void BattleMenu::moveArrow(Direction dir)
{
  State currentState = m_stateStack.top();

  if (currentState == STATE_SELECT_ACTION)
  {
    m_actionMenu->moveArrow(dir);
  }
  else if (currentState == STATE_SELECT_CHARACTER)
  {
    m_statusMenu->moveArrow(dir);
  }
  else if (currentState == STATE_SELECT_MONSTER)
  {
    m_monsterMenu->moveArrow(dir);
  }
  else if (currentState == STATE_SELECT_SPELL)
  {
    m_spellMenu->moveArrow(dir);
  }
  else if (currentState == STATE_SELECT_ITEM)
  {
    m_itemMenu->moveArrow(dir);
  }
}

void BattleMenu::draw(sf::RenderTarget& target, int x, int y)
{
  State currentState = m_stateStack.top();

  if (isVisible())
  {
    if (!m_actionMenuHidden)
    {
      draw_frame(target, x, y, config::GAME_RES_X, m_actionMenu->getHeight() + 16);
    }

    if (currentState == STATE_SELECT_MONSTER &&
        (m_actionMenu->getCurrentMenuChoice() == "Spell" ||
         m_actionMenu->getCurrentMenuChoice() == "Item"))
    {
      if (m_actionMenu->getCurrentMenuChoice() == "Spell")
      {
        draw_text_bmp(target, x + 8, y + 8, "Casting: %s", m_spellMenu->getSelectedSpell()->name.c_str());
      }
      else if (m_actionMenu->getCurrentMenuChoice() == "Item")
      {
        draw_text_bmp(target, x + 8, y + 8, "Using: %s", m_itemMenu->getSelectedItemName().c_str());
      }
    }
    else
    {
      if (!m_actionMenuHidden)
      {
        draw_text_bmp(target, x + 8, y + 8, "Action");
        draw_text_bmp(target, x + 88, y + 8, "Name");
        draw_text_bmp(target, x + 136, y + 8, "Cond");
        draw_text_bmp(target, x + 180, y + 8, "HP");
        draw_text_bmp(target, x + 216, y + 8, "MP");
      }
    }

    if (!m_actionMenuHidden)
    {
      m_actionMenu->draw(target, x, y + 24);
    }

    m_statusMenu->draw(target, x + 80, y + 24);
  }

  m_monsterMenu->draw(target, 0, 0);

  if (currentState == STATE_SELECT_SPELL && m_spellMenu->isVisible())
  {
    m_spellMenu->draw(target, 16, 16);
  }
  else if (currentState == STATE_SELECT_ITEM && m_itemMenu->isVisible())
  {
    m_itemMenu->draw(target, 16, 16);
  }
}

void BattleMenu::resetChoice()
{
  Menu::resetChoice();

  m_actionMenu->resetChoice();
  m_statusMenu->resetActor();

  // If this happens, no actors are in condition to act so skip
  // selecting actions!
  if (m_statusMenu->getCurrentActor()->incapacitated())
  {
    m_battle->doneSelectingActions();
  }
}

void BattleMenu::closeSpellMenu()
{
  delete m_spellMenu;
  m_spellMenu = 0;
}

void BattleMenu::closeItemMenu()
{
  delete m_itemMenu;
  m_itemMenu = 0;
}

Character* BattleMenu::getTarget(Target targetType) const
{
  Character* result = 0;

  if (targetType == TARGET_SINGLE_ENEMY)
  {
    result = m_monsterMenu->getCurrentMonster();
  }
  else if (targetType == TARGET_SINGLE_ALLY)
  {
    result = m_statusMenu->getCurrentSelectedActor();
  }
  else if (targetType == TARGET_ALL_ENEMY || targetType == TARGET_ALL_ALLY)
  {
    result = 0;
  }
  else if (targetType == TARGET_SELF)
  {
    result = m_statusMenu->getCurrentActor();
  }

  return result;
}

void BattleMenu::setActionMenuHidden(bool hidden)
{
  m_actionMenuHidden = hidden;
  m_statusMenu->setCurrentActorRectHidden(hidden);
}

///////////////////////////////////////////////////////////////////////////////

BattleActionMenu::BattleActionMenu()
{
  addEntry("Attack");
  addEntry("Spell");
  addEntry("Item");
  addEntry("Guard");
  addEntry("Run");

  setMaxVisible(4);
}

void BattleActionMenu::handleConfirm()
{

}

///////////////////////////////////////////////////////////////////////////////

BattleStatusMenu::BattleStatusMenu()
 : m_currentActor(0),
   m_currenActorRectHidden(false)
{
  const std::vector<PlayerCharacter*>& party = get_player()->getParty();

  for (auto it = party.begin(); it != party.end(); ++it)
  {
    addEntry((*it)->getName());
  }

  setCursorVisible(false);
}

void BattleStatusMenu::handleConfirm()
{

}

void BattleStatusMenu::draw(sf::RenderTarget& target, int x, int y)
{
  draw_frame(target, x, y, getWidth(), getHeight());

  for (int i = 0; i < getNumberOfChoice(); i++)
  {
    std::string name = getChoice(i);
    PlayerCharacter* character = get_player()->getCharacter(name);

    int offY = y + 8 + i * ENTRY_OFFSET;

    float hpPercent = (float)character->getAttribute("hp").current / (float)character->getAttribute("hp").max;

    draw_text_bmp(target, x + 8,  offY, "%s", limit_string(name, 5).c_str());
    draw_text_bmp_ex(target, x + 56, offY,
        get_status_effect(character->getStatus())->color,
        "%s", limit_string(character->getStatus(), 4).c_str());
    draw_text_bmp_ex(target, x + 100, offY,
        hpPercent > 0.2 ? sf::Color::White : sf::Color::Red,
        "%d", character->getAttribute("hp").current);
    draw_text_bmp(target, x + 136, offY, "%d", character->getAttribute("mp").current);

    if (i == m_currentActor && !m_currenActorRectHidden)
    {
      sf::RectangleShape rect = make_select_rect(x + 6, offY - 1, getWidth() - 12, 11, sf::Color::White);
      target.draw(rect);
    }

    if (cursorVisible())
    {
      if (i == getCurrentChoiceIndex())
      {
        sf::RectangleShape rect = make_select_rect(x + 6, offY - 1, getWidth() - 12, 11, sf::Color::Red);
        target.draw(rect);
      }
    }
  }
}

int BattleStatusMenu::getWidth() const
{
  return config::GAME_RES_X - 80;
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
        return false;
      }
    }
  }

  if (m_currentActor < 0)
  {
    m_currentActor = 0;
    return false;
  }

  return true;
}

bool BattleStatusMenu::nextActor()
{
  int tmpIndex = m_currentActor;

  m_currentActor++;

  if (m_currentActor < getNumberOfChoice())
  {
    while (getCurrentActor()->incapacitated())
    {
      m_currentActor++;
      if (m_currentActor >= getNumberOfChoice())
      {
        m_currentActor = tmpIndex;
        return false;
      }
    }
  }

  if (m_currentActor >= getNumberOfChoice())
  {
    m_currentActor = getNumberOfChoice() - 1;
    return false;
  }

  return true;
}

Character* BattleStatusMenu::getCurrentActor()
{
  return get_player()->getParty().at(m_currentActor);
}

Character* BattleStatusMenu::getCurrentSelectedActor()
{
  return get_player()->getParty().at(getCurrentChoiceIndex());
}

void BattleStatusMenu::resetActor()
{
  m_currentActor = 0;

  while (getCurrentActor()->incapacitated())
  {
    m_currentActor++;
    if (m_currentActor >= getNumberOfChoice())
    {
      m_currentActor = getNumberOfChoice() - 1;
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

BattleMonsterMenu::BattleMonsterMenu(const std::vector<Character*>& monsters)
 : m_monsters(monsters)
{
  setCursorVisible(false);

  for (auto it = m_monsters.begin(); it != m_monsters.end(); ++it)
  {
    addEntry((*it)->getName());
  }
}

void BattleMonsterMenu::handleConfirm()
{

}

void BattleMonsterMenu::moveArrow(Direction dir)
{
  // Just map left/right to up/down

  int prev = getCurrentChoiceIndex();

  if (dir == DIR_LEFT)
  {
    Menu::moveArrow(DIR_UP);

    while (getCurrentMonster()->getStatus() == "Dead")
    {
      setCurrentChoice(getCurrentChoiceIndex() - 1);
      if (getCurrentChoiceIndex() < 0)
      {
        setCurrentChoice(prev);
      }
    }
  }
  else if (dir == DIR_RIGHT)
  {
    Menu::moveArrow(DIR_DOWN);

    while (getCurrentMonster()->getStatus() == "Dead")
    {
      setCurrentChoice(getCurrentChoiceIndex() + 1);
      if (getCurrentChoiceIndex() >= getNumberOfChoice())
      {
        setCurrentChoice(prev);
      }
    }
  }
}

void BattleMonsterMenu::draw(sf::RenderTarget& target, int x, int y)
{
//  var posX = Math.floor(canvas.width / 2);
//  var posY = Math.floor(canvas.height / 2);
//  posX -= Math.floor(enemy.sprite.width / 2);
//  posY -= Math.floor(enemy.sprite.height / 2);
//
//  posX -= ( Math.floor((this.enemies.length / 2)) - i) *
//      (enemy.sprite.width + 12);

  for (size_t i = 0; i < m_monsters.size(); i++)
  {
    const Character* monster = m_monsters[i];

    if (monster->getStatus() == "Dead")
      continue;

    int posX = config::GAME_RES_X / 2;
    int posY = config::GAME_RES_Y / 2;

    posX -= monster->spriteWidth() / 2;
    posY -= monster->spriteHeight() / 2;

    posX -= (m_monsters.size() / 2 - i) * (monster->spriteWidth() + 12);

    monster->draw(target, posX, posY);

    if (cursorVisible() && getCurrentChoiceIndex() == (int)i)
    {
      sf::RectangleShape rect = make_select_rect(posX - 2, posY - 2, monster->spriteWidth() + 2, monster->spriteHeight() + 2);
      target.draw(rect);

      draw_frame(target, 0, 0, config::GAME_RES_X, 24);
      draw_text_bmp(target, 8, 8, "%s", get_monster_description(monster->getName()).c_str());

      draw_text_bmp(target,
          posX + monster->spriteWidth() / 2 - 8*(monster->getName().size() / 2),
          posY + monster->spriteHeight() + 4,
          "%s", monster->getName().c_str());
    }
  }
}

Character* BattleMonsterMenu::getCurrentMonster()
{
  return m_monsters[getCurrentChoiceIndex()];
}

void BattleMonsterMenu::fixSelection()
{
  if (getCurrentMonster()->getStatus() == "Dead")
  {
    for (size_t i = 0; i < m_monsters.size(); i++)
    {
      if (m_monsters[i]->getStatus() != "Dead")
      {
        setCurrentChoice(i);
        break;
      }
    }
  }
}
