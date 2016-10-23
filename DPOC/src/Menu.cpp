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
  return (4 + get_longest_string(m_menuChoices).size()) * 8;
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
