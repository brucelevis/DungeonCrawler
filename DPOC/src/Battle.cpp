#include <iterator>

#include "Config.h"
#include "Utility.h"
#include "random_pick.h"

#include "StatusEffect.h"
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

static void check_death(Character* actor)
{
  if (actor->getAttribute("hp").current <= 0)
  {
    cause_status(actor, "Dead");
  }
}

Battle::Battle(sf::RenderWindow& window, const std::vector<Character*>& monsters)
 : m_battleOngoing(false),
   m_state(STATE_SELECT_ACTIONS),
   m_battleMenu(this, monsters),
   m_monsters(monsters),
   m_currentActor(0),
   m_window(window),
   m_turnDelay(0),
   m_shakeCounter(0)
{
  m_targetTexture.create(window.getSize().x, window.getSize().y);
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

      if (m_shakeCounter > 0)
        m_shakeCounter--;

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
      else if (m_state == STATE_PROCESS_STATUS_EFFECTS)
      {
        processStatusEffects();
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
    if (get_player()->getItem(action.objectName))
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
    else
    {
      play_sound(config::SOUND_CANCEL);

      battle_message("%s tries to use %s... But there are none left!",
          m_currentActor->getName().c_str(), action.objectName.c_str());

      m_battleActions[m_currentActor].actionName = "";

      m_turnDelay = 16;
    }
  }
  else if (action.actionName == "Guard")
  {
    battle_message("%s guards.", m_currentActor->getName().c_str());
  }
  else if (action.actionName == "Run")
  {
    play_sound(config::SOUND_ESCAPE);
    show_message("You run away.");
  }

  m_state = STATE_SHOW_ACTION;
}

void Battle::showAction()
{
  if (!effectInProgress() && m_turnDelay == 0)
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
    std::string actionName = m_battleActions[m_currentActor].actionName;

    if (actionName == "Attack" || actionName == "Spell" || actionName == "Item")
    {
      Character* currentTarget = m_currentTargets.front();
      m_currentTargets.erase(m_currentTargets.begin());

      if (isMonster(currentTarget))
      {
        currentTarget->flash().start(6, 3);
      }

      int damage = 0;

      if (actionName == "Attack")
      {
        bool guard = false;

        if (m_battleActions.count(currentTarget) > 0 &&
            m_battleActions[currentTarget].actionName == "Guard")
        {
          guard = true;
        }

        damage = attack(m_currentActor, currentTarget, guard);
      }
      else if (actionName == "Spell")
      {
        const Spell* spell = get_spell(m_battleActions[m_currentActor].objectName);

        damage = cast_spell(spell, m_currentActor, currentTarget);
      }
      else if (actionName == "Item")
      {
        Item& item = item_ref(m_battleActions[m_currentActor].objectName);

        damage = use_item(&item, m_currentActor, currentTarget);
      }

      if (damage > 0)
      {
        battle_message("%s takes %d damage!", currentTarget->getName().c_str(), damage);

        if (isMonster(m_currentActor))
        {
          play_sound(config::SOUND_ENEMY_HIT);

          shakeScreen();
        }
        else
        {
          play_sound(config::SOUND_HIT);
        }
      }
      else if (damage == 0 && actionName == "Attack")
      {
        battle_message("Miss! %s takes no damage!", currentTarget->getName().c_str(), damage);

        play_sound(config::SOUND_MISS);
      }
      else if (damage < 0)
      {
        battle_message("%s is healed %d HP!", currentTarget->getName().c_str(), -damage);

        play_sound(config::SOUND_HEAL);
      }

      check_death(currentTarget);

    }

    m_turnDelay = TURN_DELAY_TIME;

    if (checkVictoryOrDefeat())
    {

    }
    else if (m_currentTargets.empty())
    {
      if (actionName == "Run")
      {
        m_state = STATE_ESCAPE;
      }
      else
      {
        m_state = STATE_EFFECT_MESSAGE;
      }
    }
  }
}

void Battle::doVictory()
{
  if (!effectInProgress() && m_turnDelay == 0)
  {
    clear_message();
    m_battleMenu.setVisible(false);

    show_message("Victory!");
    show_message("The party gains %d experience and %d gold!", getExperience(), getGold());

    get_player()->gainExperience(getExperience());
    get_player()->gainGold(getGold());

    // reset attributes that might have been affected by buffs. check for level up.
    for (auto it = get_player()->getParty().begin(); it != get_player()->getParty().end(); ++it)
    {
      reset_attribute((*it)->getAttribute("strength"));
      reset_attribute((*it)->getAttribute("power"));
      reset_attribute((*it)->getAttribute("defense"));
      reset_attribute((*it)->getAttribute("magic"));
      reset_attribute((*it)->getAttribute("mag.def"));
      reset_attribute((*it)->getAttribute("speed"));

      std::vector<StatusEffect*> statusEffects = (*it)->getStatusEffects();
      for (auto statusIt = statusEffects.begin(); statusIt != statusEffects.end(); ++statusIt)
      {
        if ((*statusIt)->battleOnly)
        {
          (*it)->cureStatus((*statusIt)->name);
        }
      }

      (*it)->checkLevelUp();
    }

    for (auto it = m_monsters.begin(); it != m_monsters.end(); ++it)
    {
      std::vector<std::string> items = monster_drop_items(get_monster_definition((*it)->getName()));

      for (auto itemIt = items.begin(); itemIt != items.end(); ++itemIt)
      {
        get_player()->addItemToInventory(*itemIt, 1);
        show_message("%s dropped %s!", (*it)->getName().c_str(), itemIt->c_str());
      }
    }

    m_state = STATE_VICTORY_POST;
  }
}

void Battle::processStatusEffects()
{
  if (m_turnDelay == 0 && !effectInProgress())
  {
    if (m_currentTargets.size() > 0)
    {
      clear_message();

      Character* current = m_currentTargets.back();
      m_currentTargets.pop_back();

      bool handledEffect = false;

      if (processStatusEffectForCharacter(current))
        handledEffect = true;

      if (handledEffect)
      {
        m_turnDelay = TURN_DELAY_TIME;
      }
    }
    else
    {
      m_state = STATE_SELECT_ACTIONS;
      m_battleMenu.setActionMenuHidden(false);
      m_battleMenu.resetChoice();

      clear_message();
    }
  }
}

bool Battle::processStatusEffectForCharacter(Character* actor)
{
  bool didProcess = false;
  bool tookDamage = false;

  if (actor->getStatus() == "Dead")
  {
    return false;
  }

  const std::vector<StatusEffect*> statusEffects = actor->getStatusEffects();

  for (auto it = statusEffects.begin(); it != statusEffects.end(); ++it)
  {
    StatusEffect* status = *it;

    if (status->damageType != StatusEffect::DAMAGE_NONE)
    {
      int damage = 0;

      if (status->damageType == StatusEffect::DAMAGE_FIXED)
      {
        damage = status->damagePerTurn;
      }
      else if (status->damageType == StatusEffect::DAMAGE_PERCENT)
      {
        float percent = (float)status->damagePerTurn / 100.0f;
        damage = percent * (float)actor->getAttribute(status->damageStat).max;
      }

      actor->takeDamage(status->damageStat, damage);

      battle_message("%s takes %d %s damage from %s!",
          actor->getName().c_str(), damage, status->damageStat.c_str(), status->name.c_str());

      if (!status->sound.empty())
        play_sound("Resources/Audio/" + status->sound);

      didProcess = true;
      tookDamage = true;
    }

    int range = random_range(0, 100);
    if (range < status->recoveryChance)
    {
      cure_status(actor, status->name);
      didProcess = true;
    }
  }

  if (tookDamage)
  {
    check_death(actor);
    checkVictoryOrDefeat();
  }

  if (didProcess)
  {
    if (isMonster(actor))
    {
      actor->flash().start(6, 3);
    }
  }

  return didProcess;
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

      if ((m_state == STATE_ESCAPE || m_state == STATE_VICTORY_POST) && message.isWaitingForKey())
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
  m_targetTexture.clear();

  // Because "clear" doesn't clear...
  sf::RectangleShape clearRect;
  clearRect.setFillColor(sf::Color::Black);
  clearRect.setSize(sf::Vector2f(m_targetTexture.getSize().x, m_targetTexture.getSize().y));
  m_targetTexture.draw(clearRect);

  int battleMenuX = 0;

  if (m_state != STATE_SELECT_ACTIONS)
  {
    battleMenuX = -40;
  }

  m_battleMenu.draw(m_targetTexture, battleMenuX, 152);

  if (Message::instance().isVisible())
  {
    Message::instance().draw(m_targetTexture);
  }

  draw_battle_message(m_targetTexture);

  for (auto it = m_activeEffects.begin(); it != m_activeEffects.end(); ++it)
  {
    (*it)->render(m_targetTexture);
  }

  m_targetTexture.display();

  sf::Sprite sprite;
  sprite.setTexture(m_targetTexture.getTexture());
  sprite.setPosition(0, 0);

  if (m_shakeCounter > 0)
  {
    sprite.setPosition(random_range(-4, 4), 0);
    // SHake in y-axis as well?!
    //sprite.setPosition(random_range(-4, 4), random_range(-4, 4));
  }

  m_window.draw(sprite);
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

  m_battleMenu.setActionMenuHidden(true);

  m_state = STATE_EXECUTE_ACTIONS;
}

void Battle::addToBattleOrder(Character* character)
{
  if (!character->incapacitated())
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

  if (m_shakeCounter > 0)
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

  while (m_battleOrder.size() > 0 && m_battleOrder.back()->incapacitated())
  {
    m_battleOrder.pop_back();
  }

  if (m_battleOrder.empty())
  {
    clear_message();

    m_currentTargets = getAllActors();
    m_state = STATE_PROCESS_STATUS_EFFECTS;
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

    effectName = item.effect;
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

std::vector<Character*> Battle::getAllActors() const
{
  std::vector<Character*> result;

  for (auto it = m_monsters.begin(); it != m_monsters.end(); ++it)
  {
    result.push_back(*it);
  }

  for (auto it = get_player()->getParty().begin(); it != get_player()->getParty().end(); ++it)
  {
    result.push_back(*it);
  }

  return result;
}

bool Battle::checkVictoryOrDefeat()
{
  if (all_dead(m_monsters))
  {
    m_state = STATE_VICTORY_PRE;

    return true;
  }
  else if (all_dead(get_player()->getParty()))
  {
    m_state = STATE_DEFEAT;

    return true;
  }

  return false;
}

void Battle::shakeScreen()
{
  m_shakeCounter = 16;
}
