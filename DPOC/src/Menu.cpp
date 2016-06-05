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
#include "Skill.h"
#include "Vocabulary.h"
#include "MenuTextHelpers.h"

#include "SaveLoad.h"
#include "SaveMenu.h"

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

static bool isOKTarget(Character* target, Target targetType)
{
  bool targetOK = !target->hasStatus("Dead");

  if (!targetOK && targetType == TARGET_DEAD)
  {
    targetOK = true;
  }
  else if (targetOK && targetType == TARGET_DEAD)
  {
    // Can't target living with TARGET_DEAD.
    targetOK = false;
  }

  return targetOK;
}

Menu::Menu()
 : m_arrowTexture(cache::loadTexture("UI/Arrow.png")),
   m_visible(false),
   m_currentMenuChoice(0),
   m_maxVisible(-1),
   m_scroll(0),
   m_cursorVisible(true)
{

}

Menu::~Menu()
{
  cache::releaseTexture(m_arrowTexture);
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
  Persistent::instance().set("$sys:choice", getCurrentChoiceIndex());
}

///////////////////////////////////////////////////////////////////////////////

MainMenu::MainMenu()
 : m_itemMenu(0),
   m_spellMenu(0),
   m_characterMenu(new CharacterMenu),
   m_equipMenu(0),
   m_saveMenu(0)
{
  addEntry("Item");
  addEntry("Spell");
  addEntry("Equip");
  addEntry("Status");
  addEntry("Skills");
  addEntry("Map");
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
  delete m_saveMenu;
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
    else if (getCurrentMenuChoice() == "Skills")
    {
      m_characterMenu->resetChoice();
      openCharacterMenu();
    }
    else if (getCurrentMenuChoice() == "Equip")
    {
      m_characterMenu->resetChoice();
      openCharacterMenu();
    }
    else if (getCurrentMenuChoice() == "Save")
    {
      openSaveMenu();
    }
    else if (getCurrentMenuChoice() == "Map")
    {
      Game::instance().openMap();
    }
  }
  else if (currentState == STATE_CHARACTER_MENU)
  {
    if (getCurrentMenuChoice() == "Spell")
    {
      if (m_characterMenu->getSpellToUse())
      {
        m_characterMenu->setTargetToCurrentChoice();

        bool targetOK = isOKTarget(m_characterMenu->getTarget(), m_characterMenu->getSpellToUse()->target);

        if (can_cast_spell(m_characterMenu->getSpellToUse(), m_characterMenu->getUser()) &&
            targetOK)
        {
          play_sound(config::get("SOUND_USE_ITEM"));

          cast_spell(m_characterMenu->getSpellToUse(),
              m_characterMenu->getUser(),
              m_characterMenu->getTarget());

          // Reduce it here since cast_spell is called for each target when
          // spell has multiple targets.
          m_characterMenu->getUser()->getAttribute(terms::mp).current -= m_characterMenu->getSpellToUse()->mpCost;
        }
        else
        {
          play_sound(config::get("SOUND_CANCEL"));
        }
      }
      else
      {
        //m_stateStack.pop();
        m_characterMenu->setUserToCurrentChoice();
        if (m_characterMenu->getUser() && !m_characterMenu->getUser()->hasStatus("Dead"))
        {
          openSpellMenu(m_characterMenu->getCurrentMenuChoice());
        }
        else
        {
          play_sound(config::get("SOUND_CANCEL"));
        }
      }
    }
    else if (getCurrentMenuChoice() == "Item")
    {
      Item* itemToUse = get_player()->getItem(m_characterMenu->getItemToUse());

      if (m_characterMenu->getItemToUse().size() > 0 && itemToUse)
      {
        m_characterMenu->setUserToCurrentChoice();
        m_characterMenu->setTargetToCurrentChoice();

        bool targetOK = isOKTarget(m_characterMenu->getTarget(),
            create_item(m_characterMenu->getItemToUse(), 1).target);

        if (m_characterMenu->getTarget() && targetOK && m_characterMenu->getUser()->canUseItemInMenu(*itemToUse))
        {
          play_sound(config::get("SOUND_USE_ITEM"));

          use_item(itemToUse, m_characterMenu->getUser(), m_characterMenu->getTarget());

          get_player()->removeItemFromInventory(m_characterMenu->getItemToUse(), 1);

          m_itemMenu->refresh();
        }
        else
        {
          play_sound(config::get("SOUND_CANCEL"));
        }

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
    else if (getCurrentMenuChoice() == "Skills")
    {
      m_stateStack.push(STATE_SKILL_MENU);
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
      play_sound(config::get("SOUND_CANCEL"));
    }
  }
  else if (currentState == STATE_ITEM_MENU)
  {
    std::string itemName = m_itemMenu->getSelectedItemName();
    Item* item = get_player()->getItem(itemName);
    if (item && (item->type == ITEM_USE || item->type == ITEM_USE_MENU) &&
        (item->target == TARGET_SINGLE_ALLY || item->target == TARGET_DEAD))
    {
      m_characterMenu->setItemToUse(itemName);
      openCharacterMenu();
    }
    else
    {
      play_sound(config::get("SOUND_CANCEL"));
    }
  }
  else if (currentState == STATE_EQUIP_MENU)
  {
    m_equipMenu->handleConfirm();
  }
  else if (currentState == STATE_SAVE_MENU)
  {
    m_saveMenu->handleConfirm();
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
      m_characterMenu->setCursorVisible(true);
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
    else if (currentState == STATE_SKILL_MENU)
    {
      m_stateStack.pop();
    }
    else if (currentState == STATE_SAVE_MENU)
    {
      m_saveMenu->handleEscape();
      if (!m_saveMenu->isVisible())
      {
        closeSaveMenu();
      }
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
  else if (currentState == STATE_CHARACTER_MENU || currentState == STATE_STATUS_MENU || currentState == STATE_SKILL_MENU)
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
  else if (currentState == STATE_SAVE_MENU)
  {
    m_saveMenu->moveArrow(dir);
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

void MainMenu::openSaveMenu()
{
  m_saveMenu = new SaveMenu(SaveMenu::SAVE);
  m_saveMenu->setVisible(true);

  m_stateStack.push(STATE_SAVE_MENU);
}

void MainMenu::closeSaveMenu()
{
  delete m_saveMenu;
  m_saveMenu = 0;

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
    const int menuChoicesWidth = 72;
    const int gpPanelHeight = 32;

    draw_frame(target, x, y, config::GAME_RES_X, config::GAME_RES_Y);

    draw_frame(target, x, y, menuChoicesWidth, getNumberOfChoice() * 16);
    draw_frame(target, x, y + 208, menuChoicesWidth, gpPanelHeight);

    draw_text_bmp(target, x + 8, y + 13*16+7, "%s", vocab_short(terms::gold).c_str());
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
    else if (currentState == STATE_SKILL_MENU)
    {
      drawSkills(target, x + 24, y + 24);
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
    else if (currentState == STATE_SAVE_MENU)
    {
      m_saveMenu->draw(target, config::GAME_RES_X / 2 - m_saveMenu->getWidth() / 2,
          config::GAME_RES_Y / 2 - m_saveMenu->getHeight() / 2);
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

  draw_hp(target, character, x + 40, y + 12);
  draw_mp(target, character, x + 40, y + 24);

  draw_text_bmp(target, x + 40 + 96, y + 12, "Lv: %d", character->computeCurrentAttribute(terms::level));
  draw_text_bmp(target, x + 40 + 96, y + 24, "Tn: %d", character->toNextLevel());

  y += 40;

  draw_stat_block(target, character, x, y);

  auto equipNames = get_equip_names();
  for (size_t i = 0; i < equipNames.size(); i++)
  {
    Item* item = character->getEquipment(equipNames[i]);
    draw_text_bmp(target, x, y + 84 + 12 * i, "%s: %s", vocab(equipNames[i]).c_str(), item ? item->name.c_str(): "");
  }
}

void MainMenu::drawSkills(sf::RenderTarget& target, int x, int y)
{
  PlayerCharacter* character = Game::instance().getPlayer()->getCharacter(m_characterMenu->getCurrentMenuChoice());

  int frameX = 16;
  int frameY = 16;
  int frameW = 14*16;
  int frameH = 13*16;

  draw_frame(target, frameX, frameY, frameW, frameH);

  character->draw(target, x, y);

  std::vector<std::string> skills = Skill::getAllSkills();

  draw_text_bmp_ex(target, x + 40, y,
      get_status_effect(character->getStatus())->color,
      "%s (%s)", character->getName().c_str(), character->getStatus().c_str());

  draw_hp(target, character, x + 40, y + 12);
  draw_mp(target, character, x + 40, y + 24);

  draw_text_bmp(target, x + 40 + 96, y + 12, "Lv: %d", character->computeCurrentAttribute(terms::level));
  draw_text_bmp(target, x + 40 + 96, y + 24, "Tn: %d", character->toNextLevel());

  y += 40;
  for (size_t i = 0; i < skills.size(); i++)
  {
    std::string& skill = skills[i];
    int skillPercent = character->getBaseAttribute(skill);

    std::string str = toString(skillPercent) + "%";

    draw_text_bmp(target, x, y + i * 12, "%s", skill.c_str(), skillPercent);
    draw_text_bmp(target, x + frameW - str.size() * 8 - 16, y + i * 12, "%s", str.c_str());
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

  int visible = m_height / 12 - 1;

  setMaxVisible(visible);
}

void ItemMenu::draw(sf::RenderTarget& target, int x, int y)
{
  const int descriptionPanelHeight = 24;
  draw_frame(target, x, y, getWidth(), descriptionPanelHeight);
  Menu::draw(target, x, y + descriptionPanelHeight);

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

EquipItemMenu::EquipItemMenu(PlayerCharacter* character, int width, int height)
 : ItemMenu(width, height),
   m_character(character)
{

}

void EquipItemMenu::refresh(const std::string& equipmentType)
{
  clear();
  m_items.clear();

  const std::vector<Item>& items = get_player()->getInventory();

  addEntry("* Remove *");

  for (auto it = items.begin(); it != items.end(); ++it)
  {
    if (equip_type_string(it->type) == equipmentType && m_character->canEquip(*it))
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

  int visible = m_height / 12 - 1;

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
  const int descriptionPanelHeight = 24;
  draw_frame(target, x, y, getWidth(), descriptionPanelHeight);
  Menu::draw(target, x, y + descriptionPanelHeight);

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

    draw_hp(target, character, offX + 40, offY + i * 48 + 12);
    draw_mp(target, character, offX + 40, offY + i * 48 + 24);

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
   m_itemMenu(new EquipItemMenu(character, 256, 104)),
   m_state(STATE_SELECT_EQUIPMENT_TYPE)
{
  for (const auto equipName : get_equip_names())
  {
    addEntry(equipName);
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

  if (currentItem && getCurrentMenuChoice() == equip_type_string(currentItem->type) && m_character->canEquip(*currentItem))
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

    play_sound(config::get("SOUND_EQUIP"));
  }
  else
  {
    play_sound(config::get("SOUND_CANCEL"));
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

    play_sound(config::get("SOUND_EQUIP"));
  }
  else
  {
    play_sound(config::get("SOUND_CANCEL"));
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
  const int nameFrameHeight = 32;

  const int leftPanelY = y + nameFrameHeight;
  const int leftPanelW = 128;
  const int leftPanelH = 80;
  const int rightPanelW = 128;

  draw_frame(target, x, leftPanelY, leftPanelW, leftPanelH);
  draw_frame(target, x + leftPanelW, leftPanelY, rightPanelW, leftPanelH);

  // Top.
  draw_frame(target, x, y, config::GAME_RES_X, nameFrameHeight);
  m_character->draw(target, x, y);
  draw_text_bmp(target, x + 36, y + 8, "%s", m_character->getName().c_str());

  m_itemMenu->draw(target, 0, 112);

  int offX = x + 8;
  int offY = y + 38;

  drawDeltas(target, offX, offY);

  offX = x + 136;

  for (int i = 0; i < getNumberOfChoice(); i++)
  {
    Item* equipment = m_character->getEquipment(getChoice(i));

    std::string typeShortName = vocab_short(getChoice(i));
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
  int newStr;
  int newLuk;
  int newDef;
  int newMag;
  int newMdf;
  int newSpd;

  if (m_state == STATE_EQUIP_ITEM)
  {
    std::string currentEquip = m_character->getEquipment(getCurrentMenuChoice()) ?
        m_character->getEquipment(getCurrentMenuChoice())->name :
        "";

    if (m_itemMenu->validChoice())
    {
      m_character->equip(getCurrentMenuChoice(), m_itemMenu->getSelectedItemName());
    }
    else
    {
      m_character->equip(getCurrentMenuChoice(), "");
    }

    newStr = m_character->computeCurrentAttribute(terms::strength);
    newDef = m_character->computeCurrentAttribute(terms::defense);
    newMag = m_character->computeCurrentAttribute(terms::magic);
    newMdf = m_character->computeCurrentAttribute(terms::magdef);
    newSpd = m_character->computeCurrentAttribute(terms::speed);
    newLuk = m_character->computeCurrentAttribute(terms::luck);

    m_character->equip(getCurrentMenuChoice(), currentEquip);
  }
  else
  {
    newStr = m_character->computeCurrentAttribute(terms::strength);
    newDef = m_character->computeCurrentAttribute(terms::defense);
    newMag = m_character->computeCurrentAttribute(terms::magic);
    newMdf = m_character->computeCurrentAttribute(terms::magdef);
    newSpd = m_character->computeCurrentAttribute(terms::speed);
    newLuk = m_character->computeCurrentAttribute(terms::luck);
  }

  draw_text_bmp(target, x, y,      "%s: %d (%d)", vocab_mid(terms::strength).c_str(), m_character->computeCurrentAttribute(terms::strength), newStr);
  draw_text_bmp(target, x, y + 12, "%s: %d (%d)", vocab_mid(terms::defense).c_str(), m_character->computeCurrentAttribute(terms::defense), newDef);
  draw_text_bmp(target, x, y + 24, "%s: %d (%d)", vocab_mid(terms::magic).c_str(), m_character->computeCurrentAttribute(terms::magic), newMag);
  draw_text_bmp(target, x, y + 36, "%s: %d (%d)", vocab_mid(terms::magdef).c_str(), m_character->computeCurrentAttribute(terms::magdef), newMdf);
  draw_text_bmp(target, x, y + 48, "%s: %d (%d)", vocab_mid(terms::speed).c_str(), m_character->computeCurrentAttribute(terms::speed), newSpd);
  draw_text_bmp(target, x, y + 60, "%s: %d (%d)", vocab_mid(terms::luck).c_str(), m_character->computeCurrentAttribute(terms::luck), newLuk);
}

///////////////////////////////////////////////////////////////////////////////

BattleMenu::BattleMenu(Battle* battle, const std::vector<Character*>& monsters)
 : m_actionMenu(new BattleActionMenu),
   m_statusMenu(new BattleStatusMenu(m_actionMenu)),
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
      PlayerCharacter* character = m_statusMenu->getCurrentActor();
      Item* weapon = character->getEquipment("weapon");

      if (weapon && weapon->target == TARGET_ALL_ENEMY)
      {
        prepareAction();
        nextActor();
      }
      else
      {
        selectMonster();
      }
    }
    else if (action == "Spell")
    {
      m_spellMenu = new SpellMenu(m_statusMenu->getCurrentActor()->getName());
      m_spellMenu->setVisible(true);

      auto it = m_spellMemory.find(m_statusMenu->getCurrentActor());
      if (it != m_spellMemory.end())
      {
        m_spellMenu->setCurrentChoice(it->second);
      }

      m_stateStack.push(STATE_SELECT_SPELL);
    }
    else if (action == "Item")
    {
      m_itemMenu = new ItemMenu;
      m_itemMenu->setVisible(true);

      auto it = m_itemMemory.find(m_statusMenu->getCurrentActor());
      if (it != m_itemMemory.end())
      {
        m_itemMenu->setCurrentChoice(it->second);
      }

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
    else if (action == "Steal")
    {
      selectMonster();
    }
    else
    {
      // Undefined action.
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
    std::string action = m_actionMenu->getCurrentMenuChoice();
    Target targetType = TARGET_NONE;
    if (action == "Spell")
    {
      targetType = m_spellMenu->getSelectedSpell()->target;
    }
    else if (action == "Item")
    {
      targetType = create_item(m_itemMenu->getSelectedItemName(), 1).target;
    }

    if (!isOKTarget(m_statusMenu->getCurrentSelectedActor(), targetType))
    {
      play_sound(config::get("SOUND_CANCEL"));
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

    m_spellMemory[m_statusMenu->getCurrentActor()] = m_spellMenu->getCurrentChoiceIndex();

    if (spell->target == TARGET_NONE || spell->mpCost > m_statusMenu->getCurrentActor()->getAttribute(terms::mp).current)
    {
      play_sound(config::get("SOUND_CANCEL"));
    }
    else if (spell->target == TARGET_SINGLE_ENEMY)
    {
      m_spellMenu->setVisible(false);
      selectMonster();
    }
    else if (spell->target == TARGET_SINGLE_ALLY || spell->target == TARGET_DEAD)
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

    m_itemMemory[m_statusMenu->getCurrentActor()] = m_itemMenu->getCurrentChoiceIndex();

    if (m_statusMenu->getCurrentActor()->canUseItemInBattle(*item))
    {
      if (item->target == TARGET_SINGLE_ENEMY)
      {
        m_itemMenu->setVisible(false);
        selectMonster();
      }
      else if (item->target == TARGET_SINGLE_ALLY || item->target == TARGET_DEAD)
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
    else
    {
      play_sound(config::get("SOUND_CANCEL"));
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
    PlayerCharacter* character = m_statusMenu->getCurrentActor();
    Item* weapon = character->getEquipment("weapon");

    if (weapon && weapon->target == TARGET_ALL_ENEMY)
    {
      battleAction.target = 0;
    }
    else
    {
      battleAction.target = m_monsterMenu->getCurrentMonster();
    }
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
  else if (action == "Steal")
  {
    battleAction.target = m_monsterMenu->getCurrentMonster();
  }
  else
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
      draw_frame(target, x, y, config::GAME_RES_X, 24);
    }
    else
    {
      draw_frame(target, x + 80, y, m_statusMenu->getWidth(), 24);
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
        draw_text_bmp(target, x + 180, y + 8, "%s", vocab_mid(terms::hp).c_str());
        draw_text_bmp(target, x + 216, y + 8, "%s", vocab_mid(terms::mp).c_str());
      }
      else
      {
        draw_text_bmp(target, x + 88, y + 8, "Name");
        draw_text_bmp(target, x + 136, y + 8, "Cond");
        draw_text_bmp(target, x + 180, y + 8, "%s", vocab_mid(terms::hp).c_str());
        draw_text_bmp(target, x + 216, y + 8, "%s", vocab_mid(terms::mp).c_str());
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
  else if (targetType == TARGET_SINGLE_ALLY || targetType == TARGET_DEAD)
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

void BattleMenu::addMonster(Character* monster)
{
  m_monsterMenu->addMonster(monster);
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

void BattleActionMenu::init(PlayerCharacter* character)
{
  clear();

  for (auto it = character->getClass().battleActions.begin();
       it != character->getClass().battleActions.end();
       ++it)
  {
    addEntry(*it);
  }
}

///////////////////////////////////////////////////////////////////////////////

BattleStatusMenu::BattleStatusMenu(BattleActionMenu* actionMenu)
 : m_currentActor(0),
   m_currenActorRectHidden(false),
   m_actionMenu(actionMenu)
{
  const std::vector<PlayerCharacter*>& party = get_player()->getParty();

  for (auto it = party.begin(); it != party.end(); ++it)
  {
    addEntry((*it)->getName());
  }

  setCursorVisible(false);
  resetActor();
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

    float hpPercent = (float)character->getAttribute(terms::hp).current / (float)character->getAttribute(terms::hp).max;

    draw_text_bmp(target, x + 8,  offY, "%s", limit_string(name, 5).c_str());
    draw_text_bmp_ex(target, x + 56, offY,
        get_status_effect(character->getStatus())->color,
        "%s", limit_string(character->getStatus(), 4).c_str());
    draw_text_bmp_ex(target, x + 100, offY,
        hpPercent > 0.2 ? sf::Color::White : sf::Color::Red,
        "%d", character->getAttribute(terms::hp).current);
    draw_text_bmp(target, x + 136, offY, "%d", character->getAttribute(terms::mp).current);

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

  if (m_currentActor < getNumberOfChoice())
  {
    while (getCurrentActor()->incapacitated())
    {
      m_currentActor++;
      if (m_currentActor >= getNumberOfChoice())
      {
        m_currentActor = tmpIndex;

        refreshActionMenu();
        return false;
      }
    }
  }

  if (m_currentActor >= getNumberOfChoice())
  {
    m_currentActor = getNumberOfChoice() - 1;

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

  refreshActionMenu();
}

void BattleStatusMenu::refreshActionMenu()
{
  m_actionMenu->init(get_player()->getParty().at(m_currentActor));
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
  (void)x;
  (void)y;

//  var posX = Math.floor(canvas.width / 2);
//  var posY = Math.floor(canvas.height / 2);
//  posX -= Math.floor(enemy.sprite.width / 2);
//  posY -= Math.floor(enemy.sprite.height / 2);
//
//  posX -= ( Math.floor((this.enemies.length / 2)) - i) *
//      (enemy.sprite.width + 12);

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

    if (cursorVisible() && getCurrentChoiceIndex() == (int)i)
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

void BattleMonsterMenu::addMonster(Character* monster)
{
  m_monsters.push_back(monster);
  addEntry(monster->getName());
}
