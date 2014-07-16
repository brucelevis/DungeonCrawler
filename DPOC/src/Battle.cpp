#include <iterator>

#include "SceneManager.h"

#include "Config.h"
#include "Utility.h"
#include "random_pick.h"
#include "Game.h"
#include "Cache.h"

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

template <typename T>
T* random_dead_character(const std::vector<T*>& actors)
{
  std::vector<T*> potentials;

  for (auto it = actors.begin(); it != actors.end(); ++it)
  {
    if ((*it)->hasStatus("Dead"))
    {
      potentials.push_back(*it);
    }
  }

  if (potentials.size() > 0)
  {
    std::random_shuffle(potentials.begin(), potentials.end());

    return potentials.front();
  }

  return 0;
}

static std::string replace_dollar_with_name(const std::string& str, const std::string& name)
{
  std::string buffer;

  for (size_t i = 0; i < str.size(); i++)
  {
    if (str[i] == '$')
    {
      buffer += name;
    }
    else
    {
      buffer += str[i];
    }
  }

  return buffer;
}

static void check_death(Character* actor)
{
  if (actor->getAttribute("hp").current <= 0)
  {
    cause_status(actor, "Dead", true);

    actor->flash().fadeOut(5);
  }
}

Battle::Battle(const std::vector<Character*>& monsters)
 : m_battleOngoing(false),
   m_state(STATE_BATTLE_BEGINS),
   m_battleMenu(this, monsters),
   m_monsters(monsters),
   m_currentActor(0),
   m_turnDelay(0),
   m_canEscape(true),
   m_battleBackground(0),
   m_battleBeginFade(1.0f)
{
}

Battle::~Battle()
{
  for (auto it = m_monsters.begin(); it != m_monsters.end(); ++it)
  {
    delete *it;
  }

  cache::releaseTexture(m_battleBackground);
}

void Battle::start(bool canEscape)
{
  clear_message();

  m_battleOngoing = true;

  m_battleMusic.openFromFile(config::res_path(config::get("MUSIC_BATTLE")));
  m_battleMusic.setVolume(50);
  m_battleMusic.setLoop(true);
  m_battleMusic.play();

  m_canEscape = canEscape;
}

void Battle::update()
{
  if (m_turnDelay > 0)
    m_turnDelay--;

  updateEffects();

  if (m_state == STATE_BATTLE_BEGINS)
  {
    m_battleBeginFade -= 0.03f;

    if (m_battleBeginFade <= 0)
    {
      m_state = STATE_SELECT_ACTIONS;
    }
  }
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
    if (!effectInProgress() && m_turnDelay == 0)
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
}

void Battle::endBattle()
{
  m_battleOngoing = false;
  m_battleMusic.stop();

  // reset attributes that might have been affected by buffs and clear status effects.
  for (auto it = get_player()->getParty().begin(); it != get_player()->getParty().end(); ++it)
  {
    reset_attribute((*it)->getAttribute("strength"));
    reset_attribute((*it)->getAttribute("defense"));
    reset_attribute((*it)->getAttribute("magic"));
    reset_attribute((*it)->getAttribute("mag.def"));
    reset_attribute((*it)->getAttribute("speed"));
    reset_attribute((*it)->getAttribute("luck"));

    std::vector<StatusEffect*> statusEffects = (*it)->getStatusEffects();
    for (auto statusIt = statusEffects.begin(); statusIt != statusEffects.end(); ++statusIt)
    {
      if ((*statusIt)->battleOnly)
      {
        (*it)->cureStatus((*statusIt)->name);
      }
    }
  }

  SceneManager::instance().fadeOut(32);
}

void Battle::executeActions()
{
  m_currentActor = m_battleOrder.back();

  Action& action = m_battleActions[m_currentActor].front();

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
//  if (isMonster(m_currentActor))
//  {
  m_currentActor->flash().start(2, 3);
//  }

  /////////////////////////////////////////////////////////////////////////////
  // Check status effects.
  if (m_currentActor->hasStatusType(STATUS_FUMBLE) && coinflip())
  {
    action.actionName = "Fumble";
    action.target = 0;
  }
  else if (m_currentActor->hasStatusType(STATUS_CONFUSE))
  {
    battle_message("%s is confused!", m_currentActor->getName().c_str());

    if (coinflip())
    {
      action.actionName = "Attack";
      action.target = m_currentActor;
    }

    if (action.target && coinflip())
    {
      if (coinflip())
      {
        action.target = selectRandomFriendlyTarget(m_currentActor);
      }
      else
      {
        action.target = selectRandomTarget(m_currentActor);
      }
    }
    else if (coinflip())
    {
      action.actionName = "Fumble";
      action.target = 0;
    }
  }
  else if (m_currentActor->hasStatusType(STATUS_SILENCE))
  {
    if (action.actionName == "Spell")
    {
      action.actionName = "Silence";
      action.target = 0;
    }
  }
  /////////////////////////////////////////////////////////////////////////////

  if (action.actionName == "Attack")
  {
    // TODO: Uncomment when effects are used.
    //if (isMonster(m_currentActor))
    //{
      play_sound(config::get("SOUND_ATTACK"));
    //}

    if (action.target)
    {
      bool regularMessage = true;

      if (!isMonster(m_currentActor))
      {
        Item* weapon = dynamic_cast<PlayerCharacter*>(m_currentActor)->getEquipment("weapon");
        if (weapon && weapon->useVerb.size() > 0)
        {
          battle_message("%s %s %s!",
              m_currentActor->getName().c_str(),
              weapon->useVerb.c_str(),
              action.target->getName().c_str());

          regularMessage = false;
        }
      }

      if (regularMessage)
      {
        battle_message("%s attacks %s!", m_currentActor->getName().c_str(), action.target->getName().c_str());
      }
    }
    else
    {
      bool regularMessage = true;

      if (!isMonster(m_currentActor))
      {
        Item* weapon = dynamic_cast<PlayerCharacter*>(m_currentActor)->getEquipment("weapon");
        if (weapon && weapon->useVerb.size() > 0)
        {
          battle_message("%s %s!",
              m_currentActor->getName().c_str(),
              weapon->useVerb.c_str());

          regularMessage = false;
        }
      }

      if (regularMessage)
      {
        battle_message("%s attacks the enemies!", m_currentActor->getName().c_str());
      }
    }
  }
  else if (action.actionName == "Spell")
  {
    play_sound(config::get("SOUND_SPELL"));

    if (action.target)
    {
      if (action.target->hasStatusType(STATUS_REFLECT))
      {
        battle_message("%s casts the %s spell at %s... but it rebounds!",
            m_currentActor->getName().c_str(),
            action.objectName.c_str(),
            action.target->getName().c_str());

        action.target = m_currentActor;
      }
      else
      {
        const Spell* spell = get_spell(action.objectName);

        if (spell->verb.empty())
        {
          battle_message("%s casts the %s spell at %s!",
              m_currentActor->getName().c_str(),
              action.objectName.c_str(),
              action.target->getName().c_str());
        }
        else
        {
          std::string use = replace_dollar_with_name(spell->verb, action.target->getName());
          battle_message("%s %s", m_currentActor->getName().c_str(), use.c_str());
        }
      }
    }
    else
    {
      const Spell* spell = get_spell(action.objectName);

      if (spell->verb.empty())
      {
        battle_message("%s casts the %s spell!",
            m_currentActor->getName().c_str(),
            action.objectName.c_str());
      }
      else
      {
        battle_message("%s %s", m_currentActor->getName().c_str(), spell->verb.c_str());
      }
    }
  }
  else if (action.actionName == "Item")
  {
    if (get_player()->getItem(action.objectName))
    {
      play_sound(config::get("SOUND_USE_ITEM"));

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
      play_sound(config::get("SOUND_CANCEL"));

      battle_message("%s tries to use %s... But there are none left!",
          m_currentActor->getName().c_str(), action.objectName.c_str());

      m_battleActions[m_currentActor].front().actionName = "";

      m_turnDelay = 16;
    }
  }
  else if (action.actionName == "Guard")
  {
    battle_message("%s guards.", m_currentActor->getName().c_str());
  }
  else if (action.actionName == "Run")
  {
    if (m_canEscape)
    {
      if (random_range(0, 10) >= 2)
      {
        play_sound(config::get("SOUND_ESCAPE"));
        m_battleMenu.setVisible(false);
        show_message("You run away.");
      }
      else
      {
        battle_message("The enemy blocks your path.");
        m_battleActions[m_currentActor].front().actionName = "";
      }
    }
    else
    {
      battle_message("There is no escaping this!");
      m_battleActions[m_currentActor].front().actionName = "";
    }
  }
  else if (action.actionName == "Fumble")
  {
    play_sound(config::get("SOUND_MISS"));
    battle_message("%s is fumbling and loses its turn!", m_currentActor->getName().c_str());
  }
  else if (action.actionName == "Silence")
  {
    play_sound(config::get("SOUND_MISS"));
    battle_message("%s can't utter a word!", m_currentActor->getName().c_str());
  }
  else if (action.actionName == "Summon")
  {
    battle_message("%s calls an ally!", m_currentActor->getName().c_str());

    Character* newMonster = Character::createMonster(action.objectName);

    m_monsters.push_back(newMonster);
    m_battleMenu.addMonster(newMonster);

    battle_message("%s appears!", newMonster->getName().c_str());
  }
  else if (action.actionName == "Ponder")
  {
    battle_message("%s ponders the situation.", m_currentActor->getName().c_str());
  }

  m_state = STATE_SHOW_ACTION;
}

void Battle::showAction()
{
  if (!effectInProgress() && m_turnDelay == 0)
  {
    m_state = STATE_ACTION_EFFECT;

    if (m_battleActions[m_currentActor].front().target)
    {
      m_currentTargets.push_back(m_battleActions[m_currentActor].front().target);

      if (m_battleActions[m_currentActor].front().actionName == "Spell")
      {
        const Spell* spell = get_spell(m_battleActions[m_currentActor].front().objectName);

        // Reduce it here since cast_spell is called for each target when
        // spell has multiple targets.
        m_currentActor->getAttribute("mp").current -= spell->mpCost;
      }
    }
    else
    {
      if (m_battleActions[m_currentActor].front().actionName == "Spell")
      {
        const Spell* spell = get_spell(m_battleActions[m_currentActor].front().objectName);

        // Reduce it here since cast_spell is called for each target when
        // spell has multiple targets.
        m_currentActor->getAttribute("mp").current -= spell->mpCost;

        setCurrentTargets(spell->target);
      }
      else if (m_battleActions[m_currentActor].front().actionName == "Item")
      {
        Item& item = item_ref(m_battleActions[m_currentActor].front().objectName);

        setCurrentTargets(item.target);
      }
      else if (m_battleActions[m_currentActor].front().actionName == "Attack")
      {
        setCurrentTargets(TARGET_ALL_ENEMY);
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
    std::string actionName = m_battleActions[m_currentActor].front().actionName;
    bool criticalHit = false;

    if (actionName == "Attack" || actionName == "Spell" || actionName == "Item")
    {
      Character* currentTarget = m_currentTargets.front();
      m_currentTargets.erase(m_currentTargets.begin());

//      if (isMonster(currentTarget))
//      {
      currentTarget->flash().start(6, 3);
//      }

      int damage = 0;

      if (actionName == "Attack")
      {
        bool guard = false;

        if (m_battleActions.count(currentTarget) > 0 &&
            m_battleActions[currentTarget].front().actionName == "Guard")
        {
          guard = true;
        }

        Item* weapon = 0;
        if (!isMonster(m_currentActor))
        {
          weapon = dynamic_cast<PlayerCharacter*>(m_currentActor)->getEquipment("Weapon");
        }

        damage = attack(m_currentActor, currentTarget, guard, weapon, criticalHit);
      }
      else if (actionName == "Spell")
      {
        const Spell* spell = get_spell(m_battleActions[m_currentActor].front().objectName);

        damage = cast_spell(spell, m_currentActor, currentTarget);
      }
      else if (actionName == "Item")
      {
        Item& item = item_ref(m_battleActions[m_currentActor].front().objectName);

        damage = use_item(&item, m_currentActor, currentTarget);
      }

      if (damage > 0)
      {
        //battle_message("%s takes %d damage!", currentTarget->getName().c_str(), damage);

        if (criticalHit)
        {
          play_sound(config::get("SOUND_CRITICAL"));

          SceneManager::instance().shakeScreen(16, 8, 8);
        }
        else if (isMonster(m_currentActor))
        {
          play_sound(config::get("SOUND_ENEMY_HIT"));

          SceneManager::instance().shakeScreen(16, 4, 0);
        }
        else
        {
          play_sound(config::get("SOUND_HIT"));
        }
      }
      else if (damage == 0 && actionName == "Attack")
      {
        //battle_message("Miss! %s takes no damage!", currentTarget->getName().c_str(), damage);

        play_sound(config::get("SOUND_MISS"));
      }
      else if (damage < 0)
      {
        //battle_message("%s is healed %d HP!", currentTarget->getName().c_str(), -damage);

        play_sound(config::get("SOUND_HEAL"));
      }

      check_death(currentTarget);

    }

    m_turnDelay = TURN_DELAY_TIME;

    if (checkVictoryOrDefeat())
    {

    }
    else if (m_currentTargets.empty())
    {
      m_battleActions[m_currentActor].erase(m_battleActions[m_currentActor].begin());

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
    m_battleMusic.stop();
    m_battleMusic.openFromFile(config::res_path(config::get("MUSIC_VICTORY")));
    m_battleMusic.setLoop(false);
    m_battleMusic.play();

    show_message("Victory!");
    show_message("The party gains %d experience and %d gold!", getExperience(), getGold());

    get_player()->gainExperience(getExperience());
    get_player()->gainGold(getGold());

    // reset attributes that might have been affected by buffs. check for level up.
    for (auto it = get_player()->getParty().begin(); it != get_player()->getParty().end(); ++it)
    {
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

  didProcess = actor->tickStatusDurations();

  const std::vector<StatusEffect*> statusEffects = actor->getStatusEffects();

  for (auto it = statusEffects.begin(); it != statusEffects.end(); ++it)
  {
    StatusEffect* status = *it;

    if (status->damageType != DAMAGE_NONE)
    {
      int damage = 0;

      if (status->damageType == DAMAGE_FIXED)
      {
        damage = status->damagePerTurn;
      }
      else if (status->damageType == DAMAGE_PERCENT)
      {
        float percent = (float)status->damagePerTurn / 100.0f;
        damage = percent * (float)actor->getAttribute(status->damageStat).max;
      }

      actor->takeDamage(status->damageStat, damage);

      actor->flash().addDamageText(toString(damage) + " [" + status->damageStat + "]", sf::Color::Red);

//      battle_message("%s takes %d %s damage from %s!",
//          actor->getName().c_str(), damage, status->damageStat.c_str(), status->name.c_str());

      if (!status->sound.empty())
        play_sound("Audio/" + status->sound);

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
//    if (isMonster(actor))
//    {
    actor->flash().start(6, 3);
//    }
  }

  return didProcess;
}

void Battle::handleEvent(sf::Event& event)
{
  switch (event.type)
  {
  case sf::Event::KeyPressed:
    handleKeyPress(event.key.code);
    break;
  default:
    break;
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

        // Haha.
        if (message.currentMessage().find("level") != std::string::npos)
        {
          m_battleMusic.stop();
          m_battleMusic.openFromFile(config::res_path(config::get("MUSIC_LEVELUP")));
          m_battleMusic.setLoop(false);
          m_battleMusic.play();
        }

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

void Battle::draw(sf::RenderTarget& target)
{
  int battleMenuX = 0;

  if (m_state != STATE_SELECT_ACTIONS && m_state != STATE_BATTLE_BEGINS)
  {
    battleMenuX = -40;
  }

  if (m_battleBackground)
  {
    sf::Sprite bgSprite;
    bgSprite.setTexture(*m_battleBackground);
    target.draw(bgSprite);
  }

  m_battleMenu.draw(target, battleMenuX, 152);
  if ((m_state != STATE_SELECT_ACTIONS && m_state != STATE_BATTLE_BEGINS) && m_battleMenu.isVisible())
  {
    for (size_t i = 0; i < get_player()->getParty().size(); i++)
    {
      PlayerCharacter* actor = get_player()->getParty()[i];

      int posX;
      int posY = config::GAME_RES_Y - 64;

      // adjust for lower portraits
      if (i == 2 || i == 3) posY += 32;

      if ((i % 2) == 0) posX = 4;
      else posX = config::GAME_RES_X - 32 - 4;

      actor->draw(target, posX, posY);

      if (actor == m_currentActor)
      {
        sf::RectangleShape rect;
        rect.setFillColor(sf::Color::Transparent);
        rect.setOutlineColor(sf::Color::Red);
        rect.setOutlineThickness(1.0f);
        rect.setPosition(posX + 1, posY + 1);
        rect.setSize(sf::Vector2f(30, 30));
        target.draw(rect);
      }
    }
  }

  if (Message::instance().isVisible())
  {
    Message::instance().draw(target);
  }

  draw_battle_message(target);

  for (auto it = m_activeEffects.begin(); it != m_activeEffects.end(); ++it)
  {
    (*it)->render(target);
  }

  if (m_state == STATE_BATTLE_BEGINS)
  {
    sf::RectangleShape fade;
    sf::Color fillColor = sf::Color::White;
    fillColor.a = (float)255 * m_battleBeginFade;
    fade.setFillColor(fillColor);
    fade.setSize(sf::Vector2f(config::GAME_RES_X, config::GAME_RES_Y));
    target.draw(fade);
  }
}

void Battle::setAction(Character* user, Action action)
{
  m_battleActions[user].clear();
  m_battleActions[user].push_back(action);
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

    for (int i = 0; i < def.numberOfAttacks; i++)
    {
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
        action.target = 0;    // Default

        if (action.actionName == "Attack")
        {
          action.target = selectRandomTarget(*it);
        }
        else if (action.actionName == "Spell")
        {
          const Spell* spell = get_spell(action.objectName);
          if (spell->target == TARGET_SINGLE_ENEMY)
          {
            action.target = selectRandomTarget(*it);
          }
          else if (spell->target == TARGET_ALL_ENEMY || spell->target == TARGET_ALL_ALLY)
          {
            action.target = 0;
          }
          else if (spell->target == TARGET_SINGLE_ALLY)
          {
            action.target = selectRandomFriendlyTarget(*it);
          }
          else if (spell->target == TARGET_SELF)
          {
            action.target = *it;
          }
          else if (spell->target == TARGET_DEAD)
          {
            action.target = random_dead_character<Character>(m_monsters);
            if (action.target == 0)
            {
              action.actionName = "Ponder";
            }
          }
        }
      }

      m_battleActions[*it].push_back(action);

      addToBattleOrder(*it);
    }
  }

  // Set some variance to turn order.
  std::map<Character*, int> tmpSpeeds;
  for (auto it = m_battleOrder.begin(); it != m_battleOrder.end(); ++it)
  {
    tmpSpeeds[*it] = (*it)->getAttribute("speed").current;

    float newSpeed = (float)(*it)->getAttribute("speed").current * rand_float(0.8f, 1.2f);
    if (newSpeed <= 1)
    {
      newSpeed = (*it)->getAttribute("speed").current;
    }

    (*it)->getAttribute("speed").current = newSpeed;
  }

  std::sort(m_battleOrder.begin(), m_battleOrder.end(), speed_comparator);

  // Restore speeds.
  for (auto it = tmpSpeeds.begin(); it != tmpSpeeds.end(); ++it)
  {
    it->first->getAttribute("speed").current = it->second;
  }

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
  for (auto it = get_player()->getParty().begin(); it != get_player()->getParty().end(); ++it)
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
    if ((*it)->flash().isFlashing() || (*it)->flash().activeEffect() || (*it)->flash().isFading())
      return true;
  }
  for (auto it = get_player()->getParty().begin(); it != get_player()->getParty().end(); ++it)
  {
    if ((*it)->flash().isFlashing() || (*it)->flash().activeEffect() || (*it)->flash().isFading())
      return true;
  }

//  if (sound_is_playing())
//    return true;

  if (m_activeEffects.size() > 0)
    return true;

  if (SceneManager::instance().isShaking())
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

  // Find provoked target.
  for (auto it = actors.begin(); it != actors.end(); ++it)
  {
    if ((*it)->hasStatusType(STATUS_PROVOKE))
    {
      return *it;
    }
  }

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
  Action& action = m_battleActions[m_currentActor].front();

  std::string effectName;

  if (action.actionName == "Attack")
  {
    if (!isMonster(m_currentActor))
    {
      Item* weapon = dynamic_cast<PlayerCharacter*>(m_currentActor)->getEquipment("Weapon");
      if (weapon)
      {
        // TODO: Uncomment when effects are used.
        //effectName = weapon->effect;
      }
      else
      {
        // TODO: Uncomment when effects are used.
        //effectName = "Effect_Hit";
      }
    }
  }
  else if (action.actionName == "Spell")
  {
//    const Spell* spell = get_spell(action.objectName);

    // TODO: Uncomment when effects are used.
    //effectName = spell->effect;
  }
  else if (action.actionName == "Item")
  {
//    Item& item = item_ref(action.objectName);

    // TODO: Uncomment when effects are used.
    //effectName = item.effect;
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

    // clear_message();
    m_battleMenu.setVisible(false);

    show_message("You have been defeated...");
    SceneManager::instance().fadeOut(128);

    return true;
  }

  return false;
}

void Battle::postFade(FadeType fadeType)
{
  if (fadeType == FADE_OUT && m_state == STATE_DEFEAT)
  {
    endBattle();
    Game::instance().close();

    SceneManager::instance().fadeIn(128);
  }
  else if (fadeType == FADE_OUT && (m_state == STATE_VICTORY_POST || m_state == STATE_ESCAPE))
  {
    close();

    Game::instance().postBattle();
  }
}

void Battle::setBattleBackground(const std::string& file)
{
  if (m_battleBackground)
  {
    cache::releaseTexture(m_battleBackground);
    m_battleBackground = 0;
  }

  if (file.size() > 0)
  {
    m_battleBackground = cache::loadTexture("Backgrounds/" + file);
  }
}
