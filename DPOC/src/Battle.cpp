#include "Config.h"
#include "Utility.h"

#include "Player.h"
#include "Character.h"
#include "Message.h"

#include "Battle.h"

static bool speed_comparator(Character* left, Character* right)
{
  int left_speed = left->computeCurrentAttribute("speed");
  int right_speed = right->computeCurrentAttribute("speed");

  if (left_speed < right_speed)
    return true;
  else if (left_speed > right_speed)
    return false;

  return coinflip();
}

Battle::Battle(sf::RenderWindow& window, const std::vector<Character*>& monsters)
 : m_battleOngoing(false),
   m_state(STATE_SELECT_ACTIONS),
   m_battleMenu(this, monsters),
   m_monsters(monsters),
   m_currentActor(0),
   m_window(window)
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

      if (m_state == STATE_EXECUTE_ACTIONS)
      {
        executeActions();
      }

      if (Message::instance().isVisible())
      {
        Message::instance().update();
      }

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
    Message::instance().clear();
    Message::instance().show(m_currentActor->getName() + " attacks " + action.target->getName() + "!");
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
  Message& message = Message::instance();

  if (message.isVisible())
  {
    if (message.isWaitingForKey())
    {
      if (m_state == STATE_SHOW_ACTION)
      {
        m_state = STATE_ACTION_EFFECT;

        Message::instance().show(m_battleActions[m_currentActor].target->getName() + " takes 1 damage!", true);
      }
      else if (m_state == STATE_ACTION_EFFECT)
      {
        if (m_battleOrder.empty())
        {
          m_state = STATE_SELECT_ACTIONS;
          m_battleMenu.setCursorVisible(true);

          Message::instance().clear();
        }
        else
        {
          m_state = STATE_EXECUTE_ACTIONS;
        }
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
