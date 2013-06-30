#include "Config.h"
#include "Utility.h"

#include "Item.h"
#include "Spell.h"
#include "Attack.h"
#include "Player.h"
#include "Character.h"
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
    action.target = selectRandomTarget(m_currentActor);
  }

  clear_message();
  if (isMonster(m_currentActor))
  {
    m_currentActor->flash().start(2, 3);
  }

  if (action.actionName == "Attack")
  {
    play_sound(config::SOUND_ATTACK);

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

  }
}

void Battle::actionEffect()
{
  if (!effectInProgress() && m_turnDelay == 0)
  {
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

    play_sound(config::SOUND_HIT);

    m_turnDelay = TURN_DELAY_TIME;

    if (allDead(m_monsters))
    {
      m_state = STATE_VICTORY_PRE;
    }
    else if (allDead(get_player()->getParty()))
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
      if (newLevel > 0)
      {
        show_message("%s has reached level %d!", (*it)->getName().c_str(), newLevel);
      }
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
    Action action;
    action.actionName = "Attack";
    action.target = selectRandomTarget(*it);

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
}

bool Battle::effectInProgress() const
{
  for (auto it = m_monsters.begin(); it != m_monsters.end(); ++it)
  {
    if ((*it)->flash().isFlashing())
      return true;
  }

  if (sound_is_playing())
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
  const std::vector<Character*>& actors =
      isMonster(actor) ?
          get_player()->getParty() :
          m_monsters;

  Character* target = 0;

  if (allDead(actors))
    return 0;

  do
  {
    int targetIndex = random_range(0, actors.size());
    target = actors[targetIndex];
  } while (target->getStatus() == "Dead");

  return target;
}

bool Battle::allDead(const std::vector<Character*>& actors) const
{
  for (auto it = actors.begin(); it != actors.end(); ++it)
  {
    if ((*it)->getStatus() != "Dead")
      return false;
  }

  return true;
}

std::vector<Character*> Battle::getAliveActors(const std::vector<Character*>& actors)
{
  std::vector<Character*> result;

  for (auto it = actors.begin(); it != actors.end(); ++it)
  {
    if ((*it)->getStatus() != "Dead")
      result.push_back(*it);
  }

  return result;
}

void Battle::setCurrentTargets(Target targetType)
{
  if (targetType == TARGET_ALL_ENEMY)
    m_currentTargets = getAliveActors(m_monsters);
  else if (targetType == TARGET_ALL_ALLY)
    m_currentTargets = getAliveActors(get_player()->getParty());
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
