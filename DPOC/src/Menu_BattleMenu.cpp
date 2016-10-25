#include <functional>

#include "Item.h"
#include "Spell.h"
#include "Sound.h"
#include "Battle.h"
#include "Target.h"
#include "Player.h"
#include "Config.h"
#include "GuiStack.h"
#include "Character.h"
#include "Vocabulary.h"
#include "PlayerCharacter.h"

#include "Frame.h"
#include "draw_text.h"

#include "Menu_ItemMenu.h"
#include "Menu_SpellMenu.h"

#include "Menu_BattleStatusMenu.h"
#include "Menu_BattleActionMenu.h"
#include "Menu_BattleMonsterMenu.h"

#include "Menu_BattleMenu.h"

namespace
{
  const int actionMenuX = 0;
  const int actionMenuY = config::RAYCASTER_RES_Y;

  const int statusMenuX = 80;
  const int statusMenuY = config::RAYCASTER_RES_Y;
}

BattleMenu::BattleMenu(Battle* battle, const std::vector<Character*>& monsters)
 : m_battle(battle),
   m_monsters(monsters),
   m_actionMenuHidden(false)
{
}

BattleMenu::~BattleMenu()
{
  m_actionMenu->close();
  m_monsterMenu->close();
  m_statusMenu->close();
}

void BattleMenu::start()
{
  auto stack = getGuiStack();

  m_actionMenu = stack->addWidget<BattleActionMenu>(
    std::bind(&BattleMenu::battleActionSelected, this, std::placeholders::_1),
    std::bind(&BattleMenu::battleActionEscape, this),
    actionMenuX, actionMenuY);

  m_monsterMenu = stack->addWidget<BattleMonsterMenu>(
    std::bind(&BattleMenu::monsterSelected, this, std::placeholders::_1),
    m_monsters);

  m_statusMenu = stack->addWidget<BattleStatusMenu>(
    std::bind(&BattleMenu::playerSelected, this, std::placeholders::_1),
    std::bind(&BattleMenu::statusMenuEscape, this),
    statusMenuX, statusMenuY);

  stack->bringToFront(this);
}

bool BattleMenu::handleInput(sf::Keyboard::Key)
{
  return true;
}

void BattleMenu::draw(sf::RenderTarget& target)
{
  const int x = 0;
  const int y = 152;

  if ((getGuiStack()->getTop() == m_monsterMenu) &&
      (m_actionMenu->getCurrentMenuChoice() == "Spell" ||
       m_actionMenu->getCurrentMenuChoice() == "Item"))
  {
    if (m_actionMenu->getCurrentMenuChoice() == "Spell")
    {
      auto spellMenu = getGuiStack()->findWidget<SpellMenu>();
      draw_text_bmp(target, x + 8, y + 8, "Casting: %s", spellMenu->getSelectedSpell()->name.c_str());
    }
    else if (m_actionMenu->getCurrentMenuChoice() == "Item")
    {
      auto itemMenu = getGuiStack()->findWidget<ItemMenu>();
      draw_text_bmp(target, x + 8, y + 8, "Using: %s", itemMenu->getSelectedItemName().c_str());
    }
  }
}

void BattleMenu::activate()
{
  getGuiStack()->bringToFront(m_actionMenu);
}

void BattleMenu::resetChoice()
{
  m_actionMenu->resetChoice();
  m_statusMenu->resetActor();

  // If this happens, no actors are in condition to act so skip
  // selecting actions!
  if (m_statusMenu->getCurrentActor()->incapacitated())
  {
    m_battle->doneSelectingActions();
  }
}

void BattleMenu::setActionMenuHidden(bool hidden)
{
  m_actionMenuHidden = hidden;
  m_actionMenu->setVisible(!hidden);
  m_statusMenu->setCurrentActorRectHidden(hidden);

  if (hidden)
  {
    m_statusMenu->setX(statusMenuX - 40);
  }
  else
  {
    m_statusMenu->setX(statusMenuX);
  }
}

void BattleMenu::addMonster(Character* monster)
{
  m_monsterMenu->addMonster(monster);
  m_monsters.push_back(monster);
}

void BattleMenu::monsterSelected(Character* monster)
{
  if (monster)
  {
    prepareAction();

    nextActor();
  }
  else
  {
    getGuiStack()->yield(m_monsterMenu);
    m_monsterMenu->setCursorVisible(false);

    reshowSpellOrItemMenu();
  }
}

void BattleMenu::playerSelected(PlayerCharacter* character)
{
  std::string action = m_actionMenu->getCurrentMenuChoice();
  Target targetType = TARGET_NONE;
  if (action == "Spell")
  {
    auto spellMenu = getGuiStack()->findWidget<SpellMenu>();

    targetType = spellMenu->getSelectedSpell()->target;
  }
  else if (action == "Item")
  {
    auto itemMenu = getGuiStack()->findWidget<ItemMenu>();

    targetType = create_item(itemMenu->getSelectedItemName(), 1).target;
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

void BattleMenu::statusMenuEscape()
{
  getGuiStack()->yield(m_statusMenu);
  m_statusMenu->setCursorVisible(false);

  reshowSpellOrItemMenu();
}

void BattleMenu::battleActionSelected(const std::string& action)
{
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
    auto spellMenu = getGuiStack()->addWidget<SpellMenu>(
      std::bind(&BattleMenu::spellSelected, this, std::placeholders::_1),
      m_statusMenu->getCurrentActor()->getName(),
      16, 16);

    auto it = m_spellMemory.find(m_statusMenu->getCurrentActor());
    if (it != m_spellMemory.end())
    {
      spellMenu->getRange().moveTo(it->second);
    }
  }
  else if (action == "Item")
  {
    auto itemMenu = getGuiStack()->addWidget<ItemMenu>(
      std::bind(&BattleMenu::itemSelected, this, std::placeholders::_1),
      16, 16);

    auto it = m_itemMemory.find(m_statusMenu->getCurrentActor());
    if (it != m_itemMemory.end())
    {
      itemMenu->getRange().moveTo(it->second);
    }
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

void BattleMenu::battleActionEscape()
{
  // Redo actions for previous actor.
  m_statusMenu->prevActor();
}

void BattleMenu::itemSelected(const std::string& itemName)
{
  auto itemMenu = getGuiStack()->findWidget<ItemMenu>();
  const Item* item = get_player()->getItem(itemName);

  m_itemMemory[m_statusMenu->getCurrentActor()] = itemMenu->getRange().getIndex();

  if (m_statusMenu->getCurrentActor()->canUseItemInBattle(*item))
  {
    if (item->target == TARGET_SINGLE_ENEMY)
    {
      itemMenu->setVisible(false);
      selectMonster();
    }
    else if (item->target == TARGET_SINGLE_ALLY || item->target == TARGET_DEAD)
    {
      itemMenu->setVisible(false);
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

void BattleMenu::spellSelected(const Spell* spell)
{
  auto spellMenu = getGuiStack()->findWidget<SpellMenu>();
  m_spellMemory[m_statusMenu->getCurrentActor()] = spellMenu->getRange().getIndex();

  if (spell->target == TARGET_NONE || spell->mpCost > m_statusMenu->getCurrentActor()->getAttribute(terms::mp).current)
  {
    play_sound(config::get("SOUND_CANCEL"));
  }
  else if (spell->target == TARGET_SINGLE_ENEMY)
  {
    spellMenu->setVisible(false);
    selectMonster();
  }
  else if (spell->target == TARGET_SINGLE_ALLY || spell->target == TARGET_DEAD)
  {
    spellMenu->setVisible(false);
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

void BattleMenu::nextActor()
{
  closeSpellMenu();
  closeItemMenu();

  m_monsterMenu->setCursorVisible(false);
  m_statusMenu->setCursorVisible(false);
  m_actionMenu->resetChoice();

  if (!m_statusMenu->nextActor())
  {
    m_actionMenu->setVisible(false);

    // If no more actors, we are done.
    m_battle->doneSelectingActions();
  }
  else
  {
    getGuiStack()->bringToFront(m_actionMenu);
  }
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
    const Spell* spell = getGuiStack()->findWidget<SpellMenu>()->getSelectedSpell();

    battleAction.objectName = spell->name;
    battleAction.target = getTarget(spell->target);
  }
  else if (action == "Item")
  {
    const Item* item = get_player()->getItem(getGuiStack()->findWidget<ItemMenu>()->getSelectedItemName());

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
  getGuiStack()->bringToFront(m_monsterMenu);
}

void BattleMenu::selectCharacter()
{
  m_statusMenu->setCursorVisible(true);
  m_statusMenu->resetChoice();
  getGuiStack()->bringToFront(m_statusMenu);
}

void BattleMenu::closeSpellMenu()
{
  if (auto spellMenu = getGuiStack()->findWidget<SpellMenu>())
  {
    spellMenu->close();
  }
}

void BattleMenu::closeItemMenu()
{
  if (auto itemMenu = getGuiStack()->findWidget<ItemMenu>())
  {
    itemMenu->close();
  }
}

Character* BattleMenu::getTarget(Target targetType) const
{
  Character* result = nullptr;

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
    result = nullptr;
  }
  else if (targetType == TARGET_SELF)
  {
    result = m_statusMenu->getCurrentActor();
  }

  return result;
}

void BattleMenu::reshowSpellOrItemMenu()
{
  auto stack = getGuiStack();

  if (auto itemMenu = stack->findWidget<ItemMenu>())
  {
    itemMenu->setVisible(true);
    stack->bringToFront(itemMenu);
  }
  else if (auto spellMenu = stack->findWidget<SpellMenu>())
  {
    spellMenu->setVisible(true);
    stack->bringToFront(spellMenu);
  }
}
