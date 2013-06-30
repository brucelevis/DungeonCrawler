#include "Config.h"
#include "Utility.h"

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
        if (!effectInProgress())
        {
          m_state = STATE_ACTION_EFFECT;
        }
      }
      else if (m_state == STATE_ACTION_EFFECT)
      {
        if (!effectInProgress())
        {
          Character* currentTarget = m_battleActions[m_currentActor].target;
          if (isMonster(currentTarget))
          {
            currentTarget->flash().start(6, 3);
          }

          play_sound(config::SOUND_HIT);

          m_state = STATE_EFFECT_MESSAGE;
          battle_message("%s takes 1 damage!", m_battleActions[m_currentActor].target->getName().c_str());

          m_turnDelay = TURN_DELAY_TIME;
        }
      }
      else if (m_state == STATE_EFFECT_MESSAGE)
      {
        if (m_turnDelay == 0)
        {
          nextActor();
        }
      }

      update_message();

      timerThen += 1000 / config::FPS;
    }

    draw();

    sf::sleep(sf::milliseconds(timerThen - timerNow));
  }
}

void Battle::executeActions()
{
  m_currentActor = m_battleOrder.back();
  m_battleOrder.pop_back();

  Action action = m_battleActions[m_currentActor];

  if (action.actionName == "Attack")
  {
    clear_message();

    if (isMonster(m_currentActor))
    {
      m_currentActor->flash().start(2, 3);
    }

    play_sound(config::SOUND_ATTACK);

    battle_message("%s attacks %s!", m_currentActor->getName().c_str(), action.target->getName().c_str());
  }

  m_state = STATE_SHOW_ACTION;
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
    if (message.isWaitingForKey())
    {
      /*if (m_state == STATE_SHOW_ACTION)
      {
        m_state = STATE_ACTION_EFFECT;
      }
      else */
      if (m_state == STATE_EFFECT_MESSAGE)
      {
        nextActor();
      }
    }
    else
    {
      message.flush();
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
    action.target = get_player()->getParty().at(rand()%(get_player()->getParty().size()));

    setAction(*it, action);

    addToBattleOrder(*it);
  }

  std::sort(m_battleOrder.begin(), m_battleOrder.end(), speed_comparator);

  m_battleMenu.resetChoice();
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
  if (m_battleOrder.empty())
  {
    m_state = STATE_SELECT_ACTIONS;
    m_battleMenu.setCursorVisible(true);

    clear_message();
  }
  else
  {
    m_state = STATE_EXECUTE_ACTIONS;
  }
}
