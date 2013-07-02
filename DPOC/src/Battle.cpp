#include <iterator>

#include "Config.h"
#include "Utility.h"
#include "random_pick.h"

#include "Monster.h"
#include "Item.h"
#include "Spell.h"
#include "Attack.h"
#include "Player.h"
#include "Character.h"
#include "PlayerCharacter.h"
#include "Message.h"
#include "Sound.h"

#include "Battle.h"

static const int TURN_DELAY_TIME = 32;

static bool speed_comparator(Character* left, Character* right)
{
  int left_speed = left->computeCurrentAttribute("speed");
  int right_speed = right->computeCurrentAttribute("speed");

  if (left_speed < right_speed)
    return true;
  else if (left_speed > right_speed)
    return false;

  return true;
}

template <typename T>
static bool all_dead(const T& actors)
{
  for (auto it = actors.begin(); it != actors.end(); ++it)
  {
    if ((*it)->getStatus() != "Dead")
      return false;
  }

  return true;
}

template <typename T>
std::vector<Character*> get_alive_actors(const T& actors)
{
  std::vector<Character*> result;

  for (auto it = actors.begin(); it != actors.end(); ++it)
  {
    if ((*it)->getStatus() != "Dead")
      result.push_back(*it);
  }

  return result;
}

Battle::Battle(sf::RenderWindow& window, const std::vector<Character*>& monsters)
 : m_battleOngoing(false),
   m_state(STATE_SELECT_ACTIONS),
   m_battleMenu(this, monsters),
   m_monsters(monsters),
   m_currentActor(0),
   m_window(window),
   m_turnDelay(0)
{

}

Battle::~Battle()
{
  for (auto it = m_monsters.begin(); it != m_monsters.end(); ++it)
  {
    delete *it;
  }
}

void Battle::start()
{
  sf::Clock clock;
  int timerThen = clock.restart().asMilliseconds();

  m_battleOngoing = true;

  while (m_window.isOpen() && m_battleOngoing)
  {
    int timerNow = clock.getElapsedTime().asMilliseconds();

    while (timerThen < timerNow)
    {
      pollEvents();

      if (m_turnDelay > 0)
        m_turnDelay--;

      updateEffects();

      if (m_state == STATE_EXECUTE_ACTIONS)
      {
        executeActions();
      }
      else if (m_state == STATE_SHOW_ACTION)
      {
        showAction();
      }
      else if (m_state == STATE_ACTION_EFFECT)
      {
        actionEffect();
      }
      else if (m_state == STATE_EFFECT_MESSAGE)
      {
        if (m_turnDelay == 0)
        {
          nextActor();
        }
      }
      else if (m_state == STATE_VICTORY_PRE)
      {
        doVictory();
      }
      else if (m_state == STATE_DEFEAT)
      {

      }

      update_message();

      timerThen += 1000 / config::FPS;
    }

    draw();

    sf::sleep(sf::milliseconds(timerThen - timerNow));
  }
}

void Battle::endBattle()
{
  m_battleOngoing = false;
}

void Battle::executeActions()
{
  m_currentActor = m_battleOrder.back();

  Action& action = m_battleActions[m_currentActor];

  if (action.target && action.target->getStatus() == "Dead")
  {
    if (action.actionName == "Spell")
    {
      const Spell* spell = get_spell(action.objectName);
      if (spell->target == TARGET_SINGLE_ALLY)
        action.target = selectRandomFriendlyTarget(m_currentActor);
      else if (spell->target == TARGET_SINGLE_ENEMY)
        action.target = selectRandomTarget(m_currentActor);
    }
    else if (action.actionName == "Item")
    {
      Item& item = item_ref(action.objectName);
      if (item.target == TARGET_SINGLE_ALLY)
        action.target = selectRandomFriendlyTarget(m_currentActor);
      else if (item.target == TARGET_SINGLE_ENEMY)
        action.target = selectRandomTarget(m_currentActor);
    }
    else
    {
      action.target = selectRandomTarget(m_currentActor);
    }
  }

  clear_message();
  if (isMonster(m_currentActor))
  {
    m_currentActor->flash().start(2, 3);
  }

  if (action.actionName == "Attack")
  {
    if (isMonster(m_currentActor))
    {
      play_sound(config::SOUND_ATTACK);
    }

    battle_message("%s attacks %s!", m_currentActor->getName().c_str(), action.target->getName().c_str());
  }
  else if (action.actionName == "Spell")
  {
    play_sound(config::SOUND_SPELL);

    const Spell* spell = get_spell(action.objectName);

    m_currentActor->getAttribute("mp").current -= spell->mpCost;

    if (action.target)
    {
      battle_message("%s casts the %s spell at %s!",
          m_currentActor->getName().c_str(),
          action.objectName.c_str(),
          action.target->getName().c_str());
    }
    else
    {
      battle_message("%s casts the %s spell!",
          m_currentActor->getName().c_str(),
          action.objectName.c_str());
    }
  }
  else if (action.actionName == "Item")
  {
    play_sound(config::SOUND_USE_ITEM);

    Item& item = item_ref(action.objectName);
    get_player()->removeItemFromInventory(action.objectName, 1);

    if (action.target)
    {
      battle_message("%s uses %s on %s!",
          m_currentActor->getName().c_str(),
          item.name.c_str(),
          action.target->getName().c_str());
    }
    else
    {
      battle_message("%s uses %s!",
          m_currentActor->getName().c_str(),
          item.name.c_str());
    }
  }

  m_state = STATE_SHOW_ACTION;
}

void Battle::showAction()
{
  if (!effectInProgress())
  {
    m_state = STATE_ACTION_EFFECT;

    if (m_battleActions[m_currentActor].target)
    {
      m_currentTargets.push_back(m_battleActions[m_currentActor].target);
    }
    else
    {
      if (m_battleActions[m_currentActor].actionName == "Spell")
      {
        const Spell* spell = get_spell(m_battleActions[m_currentActor].objectName);

        setCurrentTargets(spell->target);
      }
      else if (m_battleActions[m_currentActor].actionName == "Item")
      {
        Item& item = item_ref(m_battleActions[m_currentActor].objectName);

        setCurrentTargets(item.target);
      }
    }

    createEffects();

    m_turnDelay = 16;
  }
}

void Battle::actionEffect()
{
  if (!effectInProgress() && m_turnDelay == 0)
  {
    clear_message();

    Character* currentTarget = m_currentTargets.front();
    m_currentTargets.erase(m_currentTargets.begin());

    if (isMonster(currentTarget))
    {
      currentTarget->flash().start(6, 3);
    }

    int damage = calculate_physical_damage(m_currentActor, currentTarget);

    battle_message("%s takes %d damage!", currentTarget->getName().c_str(), damage);

    currentTarget->getAttribute("hp").current -= damage;
    if (currentTarget->getAttribute("hp").current <= 0)
    {
      currentTarget->setStatus("Dead");
      battle_message("%s has fallen!", currentTarget->getName().c_str());
    }

    if (isMonster(m_currentActor))
    {
      play_sound(config::SOUND_ENEMY_HIT);
    }
    else
    {
      play_sound(config::SOUND_HIT);
    }

    m_turnDelay = TURN_DELAY_TIME;

    if (all_dead(m_monsters))
    {
      m_state = STATE_VICTORY_PRE;
    }
    else if (all_dead(get_player()->getParty()))
    {
      m_state = STATE_DEFEAT;
    }
    else if (m_currentTargets.empty())
    {
      m_state = STATE_EFFECT_MESSAGE;
    }
  }
}

void Battle::doVictory()
{
  if (!effectInProgress() && m_turnDelay == 0)
  {
    clear_message();
    show_message("Victory!");
    show_message("The party gains %d experience and %d gold!", getExperience(), getGold());

    get_player()->gainExperience(getExperience());
    get_player()->gainGold(getGold());

    // reset attributes that might have been affected by buffs. check for level up.
    for (auto it = get_player()->getParty().begin(); it != get_player()->getParty().end(); ++it)
    {
      clamp_attribute((*it)->getAttribute("strength"));
      clamp_attribute((*it)->getAttribute("power"));
      clamp_attribute((*it)->getAttribute("defense"));
      clamp_attribute((*it)->getAttribute("magic"));
      clamp_attribute((*it)->getAttribute("mag.def"));
      clamp_attribute((*it)->getAttribute("speed"));

      int newLevel = (*it)->checkLevelUp();
    }

    m_state = STATE_VICTORY_POST;
  }
}

void Battle::pollEvents()
{
  sf::Event event;

  while (m_window.pollEvent(event))
  {
    switch (event.type)
    {
    case sf::Event::Closed:
      m_window.close();
      break;
    case sf::Event::KeyPressed:
      handleKeyPress(event.key.code);

      break;
    default:
      break;
    }
  }
}

void Battle::handleKeyPress(sf::Keyboard::Key key)
{
  if (effectInProgress())
    return;

  Message& message = Message::instance();

  if (message.isVisible())
  {
    if (key == sf::Keyboard::Space)
    {
      m_turnDelay = 0;

      if (m_state == STATE_VICTORY_POST && message.isWaitingForKey())
      {
        message.nextPage();

        if (!message.isVisible())
        {
          endBattle();
        }
      }
      else if (m_state == STATE_VICTORY_POST)
      {
        message.flush();
      }
    }
  }
  else if (m_state == STATE_SELECT_ACTIONS)
  {
    if (key == sf::Keyboard::Up) m_battleMenu.moveArrow(DIR_UP);
    else if (key == sf::Keyboard::Down) m_battleMenu.moveArrow(DIR_DOWN);
    else if (key == sf::Keyboard::Left) m_battleMenu.moveArrow(DIR_LEFT);
    else if (key == sf::Keyboard::Right) m_battleMenu.moveArrow(DIR_RIGHT);
    else if (key == sf::Keyboard::Space) m_battleMenu.handleConfirm();
    else if (key == sf::Keyboard::Escape) m_battleMenu.handleEscape();
  }
}

void Battle::draw()
{
  m_window.clear();

  m_battleMenu.draw(m_window, 0, 152);

  if (Message::instance().isVisible())
  {
    Message::instance().draw(m_window);
  }

  for (auto it = m_activeEffects.begin(); it != m_activeEffects.end(); ++it)
  {
    (*it)->render(m_window);
  }

  m_window.display();
}

void Battle::setAction(Character* user, Action action)
{
  m_battleActions[user] = action;
}

void Battle::doneSelectingActions()
{
  for (auto it = get_player()->getParty().begin(); it != get_player()->getParty().end(); ++it)
  {
    addToBattleOrder(*it);
  }

  for (auto it = m_monsters.begin(); it != m_monsters.end(); ++it)
  {
    MonsterDef def = get_monster_definition((*it)->getName());

    Action action;

    if (def.actions.empty())
    {
      action.actionName = "Attack";
      action.target = selectRandomTarget(*it);
    }
    else
    {
      std::vector< rnd::random_pick_entry_t<size_t> > actionEntries;
      for (size_t i = 0; i < def.actions.size(); i++)
      {
        rnd::random_pick_entry_t<size_t> entry = { i, def.actions[i].weight };

        actionEntries.push_back(entry);
      }
      size_t actionIndex = rnd::random_pick(actionEntries);

      action.actionName = def.actions[actionIndex].action;
      action.objectName = def.actions[actionIndex].objectName;

			if (action.actionName == "Attack")
			{
			  action.target = selectRandomTarget(*it);
			}
      else if (action.actionName == "Spell")
      {
        const Spell* spell = get_spell(action.objectName);
        if (spell->target == TARGET_SINGLE_ENEMY)
          action.target = selectRandomTarget(*it);
        else if (spell->target == TARGET_ALL_ENEMY || spell->target == TARGET_ALL_ALLY)
          action.target = 0;
        else if (spell->target == TARGET_SINGLE_ALLY)
          action.target = selectRandomFriendlyTarget(*it);
        else if (spell->target == TARGET_SELF)
          action.target = *it;
      }
    }

    setAction(*it, action);

    addToBattleOrder(*it);
  }

  std::sort(m_battleOrder.begin(), m_battleOrder.end(), speed_comparator);

  m_battleMenu.setCursorVisible(false);

  m_state = STATE_EXECUTE_ACTIONS;
}

void Battle::addToBattleOrder(Character* character)
{
  if (character->getStatus() != "Dead")
  {
    m_battleOrder.push_back(character);
  }
}

void Battle::updateEffects()
{
  for (auto it = m_monsters.begin(); it != m_monsters.end(); ++it)
  {
    (*it)->flash().update();
  }

  for (auto it = m_activeEffects.begin(); it != m_activeEffects.end();)
  {
    (*it)->update();
    if ((*it)->complete())
    {
    	delete *it;
      it = m_activeEffects.erase(it);
    }
    else
    {
      ++it;
    }
  }
}

bool Battle::effectInProgress() const
{
  for (auto it = m_monsters.begin(); it != m_monsters.end(); ++it)
  {
    if ((*it)->flash().isFlashing() || (*it)->flash().activeEffect())
      return true;
  }

  if (sound_is_playing())
    return true;

  if (m_activeEffects.size() > 0)
    return true;

  return false;
}

bool Battle::isMonster(Character* actor)
{
  for (auto it = m_monsters.begin(); it != m_monsters.end(); ++it)
  {
    if (actor == *it)
      return true;
  }

  return false;
}

void Battle::nextActor()
{
  m_battleOrder.pop_back();

  while (m_battleOrder.size() > 0 && m_battleOrder.back()->getStatus() == "Dead")
  {
    m_battleOrder.pop_back();
  }

  if (m_battleOrder.empty())
  {
    m_state = STATE_SELECT_ACTIONS;
    m_battleMenu.setCursorVisible(true);
    m_battleMenu.resetChoice();

    clear_message();
  }
  else
  {
    m_state = STATE_EXECUTE_ACTIONS;
  }
}

Character* Battle::selectRandomTarget(Character* actor)
{
  std::vector<Character*> actors;

  if (isMonster(actor))
  {
    std::copy(get_player()->getParty().begin(), get_player()->getParty().end(), std::back_inserter(actors));
  }
  else
  {
    std::copy(m_monsters.begin(), m_monsters.end(), std::back_inserter(actors));
  }

  Character* target = 0;

  if (all_dead(actors))
    return 0;

  do
  {
    int targetIndex = random_range(0, actors.size());
    target = actors[targetIndex];
  } while (target->getStatus() == "Dead");

  return target;
}

Character* Battle::selectRandomFriendlyTarget(Character* actor)
{
  std::vector<Character*> actors;

  if (isMonster(actor))
  {
    std::copy(m_monsters.begin(), m_monsters.end(), std::back_inserter(actors));
  }
  else
  {
    std::copy(get_player()->getParty().begin(), get_player()->getParty().end(), std::back_inserter(actors));
  }

  Character* target = 0;

  if (all_dead(actors))
    return 0;

  do
  {
    int targetIndex = random_range(0, actors.size());
    target = actors[targetIndex];
  } while (target->getStatus() == "Dead");

  return target;
}

void Battle::setCurrentTargets(Target targetType)
{
  if (!isMonster(m_currentActor))
  {
    if (targetType == TARGET_ALL_ENEMY)
      m_currentTargets = get_alive_actors(m_monsters);
    else if (targetType == TARGET_ALL_ALLY)
      m_currentTargets = get_alive_actors(get_player()->getParty());
  }
  else
  {
    if (targetType == TARGET_ALL_ENEMY)
      m_currentTargets = get_alive_actors(get_player()->getParty());
    else if (targetType == TARGET_ALL_ALLY)
      m_currentTargets = get_alive_actors(m_monsters);
  }
}

void Battle::createEffects()
{
  Action& action = m_battleActions[m_currentActor];

  std::string effectName;

  if (action.actionName == "Attack")
  {
    if (!isMonster(m_currentActor))
    {
      Item* weapon = dynamic_cast<PlayerCharacter*>(m_currentActor)->getEquipment("Weapon");
      if (weapon)
      {
        effectName = weapon->effect;
      }
      else
      {
        effectName = "Effect_Hit";
      }
    }
  }
  else if (action.actionName == "Spell")
  {
    const Spell* spell = get_spell(action.objectName);

    effectName = spell->effect;
  }
  else if (action.actionName == "Item")
  {
    Item& item = item_ref(action.objectName);
  }

  for (auto it = m_currentTargets.begin(); it != m_currentTargets.end(); ++it)
  {
    if (isMonster(*it) && !effectName.empty())
    {
      (*it)->flash().startEffect(effectName);
    }
    else if (!effectName.empty())
    {
      // This effect will be heard but not seen.
      m_activeEffects.push_back(Effect::createEffect(effectName, -100, -100));
    }
  }
}

int Battle::getExperience() const
{
  int sum = 0;

  for (auto it = m_monsters.begin(); it != m_monsters.end(); ++it)
  {
    sum += (*it)->getAttribute("exp").current;
  }

  return sum;
}

int Battle::getGold() const
{
  int sum = 0;

  for (auto it = m_monsters.begin(); it != m_monsters.end(); ++it)
  {
    sum += (*it)->getAttribute("gold").current;
  }

  return sum;
}
