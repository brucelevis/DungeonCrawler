#include <algorithm>
#include <sstream>

#include "SceneManager.h"

#include "Sound.h"
#include "Utility.h"
#include "Map.h"
#include "Player.h"
#include "Game.h"
#include "Persistent.h"
#include "Message.h"
#include "logger.h"
#include "Config.h"
#include "EntityDef.h"
#include "Entity.h"

static bool exec_bool_operation(const std::string& operation, int lhs, int rhs)
{
  if (operation == "==") return lhs == rhs;
  if (operation == "!=") return lhs != rhs;
  if (operation == "<") return lhs < rhs;
  if (operation == ">") return lhs > rhs;
  if (operation == "<=") return lhs <= rhs;
  if (operation == ">=") return lhs >= rhs;

  TRACE("Unknown operation '%s'", operation.c_str());

  return false;
}

Entity::Entity()
: x(0),
  y(0),
  m_sprite(0),
  m_direction(DIR_DOWN),
  m_speed(0.1),
  m_targetX(0), m_targetY(0),
  m_state(STATE_NORMAL),
  m_waitCounter(0),
  m_walkThrough(false),
  m_visible(true),
  m_fixedDirection(false)
{
}

Entity::Entity(const std::string& name)
 : x(0),
   y(0),
   m_sprite(0),
   m_direction(DIR_DOWN),
   m_speed(0.1),
   m_targetX(0), m_targetY(0),
   m_state(STATE_NORMAL),
   m_waitCounter(0),
   m_walkThrough(false),
   m_visible(true),
   m_fixedDirection(false)
{
  auto it = std::find_if(ENTITY_DEF.begin(), ENTITY_DEF.end(),
      [=](const EntityDef& entity)
      {
        return entity.name == name;
      });

  if (it != ENTITY_DEF.end())
  {
    m_name = name;
    if (!it->spriteSheet.empty())
    {
      m_sprite = new Sprite;
      m_sprite->create(it->spriteSheet, it->spriteSheetX, it->spriteSheetY);
    }

    if (!it->scriptFile.empty())
    {
      m_script.loadFromFile(it->scriptFile);
    }

    if (!it->stepScriptFile.empty())
    {
      if (m_stepScript.loadFromFile(it->stepScriptFile))
      {
        m_stepScript.execute();
      }
    }

    m_speed = it->walkSpeed;
  }
  else
  {
    TRACE("Unable to create entity %s! Not found in def array.", name.c_str());
  }
}

Entity::~Entity()
{
  delete m_sprite;
}

void Entity::loadScripts(const std::string& talkScript, const std::string& stepScript)
{
  TRACE("Entity[%s]::loadScripts(%s, %s)", getTag().c_str(), talkScript.c_str(), stepScript.c_str());

  if (!talkScript.empty())
  {
    std::istringstream ss(talkScript);
    m_script.loadFromLines(get_lines(ss));
  }

  if (!stepScript.empty())
  {
    std::istringstream ss(stepScript);
    m_stepScript.loadFromLines(get_lines(ss));
    m_stepScript.execute();
  }
}

void Entity::update()
{
  if (m_state == STATE_WALKING)
  {
    walk();

    if (m_sprite)
    {
      m_sprite->update(m_direction);
    }
  }
  else if (m_state == STATE_WAITING)
  {
    m_waitCounter--;
    if (m_waitCounter == 0)
    {
      m_state = STATE_NORMAL;
    }
  }

  if (m_scriptWaitMap[&m_script] > 0)
  {
    m_scriptWaitMap[&m_script]--;
  }

  if (m_scriptWaitMap[&m_stepScript] > 0)
  {
    m_scriptWaitMap[&m_stepScript]--;
  }

  if (m_state != STATE_WALKING)
  {
    if (m_script.active() && m_scriptWaitMap[&m_script] == 0)
    {
      executeScriptLine(m_script.getCurrentData(), m_script);

      m_script.advance();
    }

    if (!m_script.active() && m_stepScript.active() && m_scriptWaitMap[&m_stepScript] == 0)
    {
      executeScriptLine(m_stepScript.getCurrentData(), m_stepScript);

      m_stepScript.advance();

      if (!m_stepScript.active())
      {
        m_stepScript.execute();
      }
    }
  }
}

void Entity::setDirection(Direction dir)
{
  if (m_fixedDirection)
    return;

  if (dir == DIR_RANDOM)
  {
    m_direction = (Direction)(rand()%4);
  }
  else
  {
    m_direction = dir;
  }

  m_sprite->setDirection(m_direction);
}

void Entity::step(Direction dir)
{
  if (m_state != STATE_WALKING)
  {
    setDirection(dir);

    if (m_direction == DIR_RIGHT)
      m_targetX = x + 1;
    else if (m_direction == DIR_LEFT)
      m_targetX = x - 1;
    else if (m_direction == DIR_DOWN)
      m_targetY = y + 1;
    else if (m_direction == DIR_UP)
      m_targetY = y - 1;

    if (!m_walkThrough && (Game::instance().getCurrentMap()->blocking(m_targetX, m_targetY) || checkPlayerCollision() || checkEntityCollision()))
    {
      m_targetX = x;
      m_targetY = y;
    }
    else
    {
      m_state = STATE_WALKING;
    }
  }
}

void Entity::walk()
{
  if (m_direction == DIR_RIGHT)
  {
    x += m_speed;
    if (x >= m_targetX)
    {
      x = m_targetX;
      m_state = STATE_NORMAL;
    }
  }
  else if (m_direction == DIR_LEFT)
  {
    x -= m_speed;
    if (x <= m_targetX)
    {
      x = m_targetX;
      m_state = STATE_NORMAL;
    }
  }
  else if (m_direction == DIR_DOWN)
  {
    y += m_speed;
    if (y >= m_targetY)
    {
      y = m_targetY;
      m_state = STATE_NORMAL;
    }
  }
  else if (m_direction == DIR_UP)
  {
    y -= m_speed;
    if (y <= m_targetY)
    {
      y = m_targetY;
      m_state = STATE_NORMAL;
    }
  }
}

void Entity::wait(int duration)
{
  if (m_state != STATE_WALKING)
  {
    m_state = STATE_WAITING;
    m_waitCounter = duration;
  }
}

void Entity::draw(sf::RenderTarget& target, const coord_t& view)
{
  if (m_sprite && m_visible)
  {
    m_sprite->render(target, getRealX() - view.x, getRealY() - view.y);
  }
}

float Entity::getRealX() const
{
  return x * config::TILE_W;
}

float Entity::getRealY() const
{
  return y * config::TILE_H;
}

void Entity::interact(const Entity* interactor)
{
  if (interactor)
  {
    face(interactor);
  }

  if (m_script.isLoaded() && !m_script.active())
  {
    m_script.execute();
  }
}

void Entity::face(const Entity* entity)
{
  switch (entity->m_direction)
  {
  case DIR_RIGHT:
    setDirection(DIR_LEFT);
    break;
  case DIR_DOWN:
    setDirection(DIR_UP);
    break;
  case DIR_LEFT:
    setDirection(DIR_RIGHT);
    break;
  case DIR_UP:
    setDirection(DIR_DOWN);
    break;
  default:
    break;
  }
}

bool Entity::canInteractWith(const Entity* interactor) const
{
  if (!isWalking() && !interactor->isWalking())
  {
    int myX = x;
    int myY = y;

    int px = interactor->x;
    int py = interactor->y;

    switch (interactor->m_direction)
    {
    case DIR_RIGHT:
      return (px == myX - 1) && (py == myY);
    case DIR_LEFT:
      return (px == myX + 1) && (py == myY);
    case DIR_DOWN:
      return (px == myX) && (py == myY - 1);
    case DIR_UP:
      return (px == myX) && (py == myY + 1);
    default:
      break;
    }
  }

  return false;
}

void Entity::executeScriptLine(const Script::ScriptData& data, Script& executingScript)
{
  if (data.opcode == Script::OP_MESSAGE)
  {
    Message::instance().show(data.data.messageData.message);

    Script::ScriptData next;
    if (executingScript.peekNext(next))
    {
      if (next.opcode == Script::OP_MESSAGE || next.opcode == Script::OP_CHOICE)
      {
        executingScript.advance();
        executeScriptLine(next, executingScript);
      }
    }
  }
  else if (data.opcode == Script::OP_WALK)
  {
    step(data.data.walkData.dir);
  }
  else if (data.opcode == Script::OP_SET_DIR)
  {
    // Script should override fixed direction property.
    bool fixTemp = m_fixedDirection;
    setFixedDirection(false);

    setDirection(data.data.walkData.dir);

    setFixedDirection(fixTemp);
  }
  else if (data.opcode == Script::OP_WAIT)
  {
    // wait(data.data.waitData.duration);
    m_scriptWaitMap[&executingScript] = data.data.waitData.duration;
  }
  else if (data.opcode == Script::OP_SET_GLOBAL)
  {
    Persistent<int>::instance().set(data.data.setGlobalData.key, data.data.setGlobalData.value);
  }
  else if (data.opcode == Script::OP_SET_LOCAL)
  {
    Persistent<int>::instance().set(getTag() + "@@" + data.data.setLocalData.key, data.data.setLocalData.value);
  }
  else if (data.opcode == Script::OP_IF)
  {
    std::string lhs = data.data.ifData.lhs;
    std::string rhs = data.data.ifData.rhs;
    std::string lhsKey = data.data.ifData.lhsKey;
    std::string rhsKey = data.data.ifData.rhsKey;
    std::string operation = data.data.ifData.boolOperation;

    int lhsValue, rhsValue;

    getIfValue(lhs, lhsKey, lhsValue);
    getIfValue(rhs, rhsKey, rhsValue);

    bool result = exec_bool_operation(operation, lhsValue, rhsValue);

    if (!result)
    {
      // Find matching end_if or else, in case the expression was false.
      int ifCount = 1;
      while (ifCount > 0)
      {
        executingScript.advance();
        if (executingScript.getCurrentData().opcode == Script::OP_IF)
        {
          ifCount++;
        }
        else if (executingScript.getCurrentData().opcode == Script::OP_END_IF)
        {
          ifCount--;
        }
        else if (executingScript.getCurrentData().opcode == Script::OP_ELSE && ifCount == 1)
        {
          // IfCount == 1 means it is the original IF, so enter this ELSE branch.
          ifCount--;
        }
      }
    }
  }
  else if (data.opcode == Script::OP_ELSE)
  {
    // In case we advanced into an ELSE opcode, continue until we find the matching END.
    int ifCount = 1;
    while (ifCount > 0)
    {
      executingScript.advance();
      if (executingScript.getCurrentData().opcode == Script::OP_IF)
      {
        ifCount++;
      }
      else if (executingScript.getCurrentData().opcode == Script::OP_END_IF)
      {
        ifCount--;
      }
    }
  }
  else if (data.opcode == Script::OP_CHOICE)
  {
    std::vector<std::string> choices;

    for (int i = 0; i < data.data.choiceData.numberOfChoices; i++)
    {
      choices.push_back(replace_string(data.data.choiceData.choices[i], '_', ' '));
    }

    Game::instance().openChoiceMenu(choices);
  }
  else if (data.opcode == Script::OP_SET_TILE_ID)
  {
    int tileId = data.data.setTileIdData.tileId;

    TileSprite* tileSprite = dynamic_cast<TileSprite*>(m_sprite);
    if (tileSprite)
    {
      tileSprite->setTileNum(tileId);
    }
    else
    {
      TRACE("Attempting to call set_tile_id on entity not using tileSprite.");
    }
  }
  else if (data.opcode == Script::OP_GIVE_ITEM)
  {
    int amount = data.data.giveItemData.amount;
    std::string itemName = data.data.giveItemData.itemName;

    get_player()->addItemToInventory(itemName, amount);
  }
  else if (data.opcode == Script::OP_TAKE_ITEM)
  {
    int amount = data.data.giveItemData.amount;
    std::string itemName = data.data.giveItemData.itemName;

    get_player()->removeItemFromInventory(itemName, amount);
  }
  else if (data.opcode == Script::OP_GIVE_GOLD)
  {
    int amount = data.data.giveGoldData.amount;
    get_player()->gainGold(amount);
  }
  else if (data.opcode == Script::OP_TAKE_GOLD)
  {
    int amount = data.data.giveGoldData.amount;
    get_player()->removeGold(amount);
  }
  else if (data.opcode == Script::OP_PLAY_SOUND)
  {
    std::string sound = data.data.playSoundData.sound;
    play_sound("Resources/Audio/" + sound);
  }
  else if (data.opcode == Script::OP_ADD_PARTY_MEMBER)
  {
    int level = data.data.addPartyMemberData.level; // TODO
    std::string name = data.data.addPartyMemberData.name;
    std::string className = data.data.addPartyMemberData.className;

    int x = get_player()->getTrain().back()->x;
    int y = get_player()->getTrain().back()->y;

    get_player()->addNewCharacter(name, className, x, y, level);
  }
  else if (data.opcode == Script::OP_REMOVE_PARTY_MEMBER)
  {
    std::string name = data.data.removePartyMemberData.name;

    get_player()->removeCharacter(name);
  }
  else if (data.opcode == Script::OP_SET_VISIBLE)
  {
    m_visible = data.data.setVisibleData.visibility;
  }
  else if (data.opcode == Script::OP_SET_WALKTHROUGH)
  {
    m_walkThrough = data.data.setWalkthroughData.walkthrough;
  }
  else if (data.opcode == Script::OP_ENABLE_CONTROLS)
  {
    get_player()->setControlsEnabled(data.data.enableControlsData.enabled);
  }
  else if (data.opcode == Script::OP_RECOVER_ALL)
  {
    get_player()->recoverAll();
  }
  else if (data.opcode == Script::OP_COMBAT)
  {
    std::vector<std::string> monsters;

    for (int i = 0; i < data.data.combatData.number; i++)
    {
      monsters.push_back(data.data.combatData.monsters[i]);
    }

    Game::instance().startBattle(monsters);
  }
  else if (data.opcode == Script::OP_END_GAME)
  {
    Game::instance().close();
    SceneManager::instance().fadeIn(128);
  }
  else if (data.opcode == Script::OP_SET_CONFIG)
  {
    config::set(data.data.setConfigData.key, data.data.setConfigData.value);
  }
  else if (data.opcode == Script::OP_TRANSFER)
  {
    std::string targetMap = data.data.transferData.targetMap;
    Game::instance().prepareTransfer(targetMap, data.data.transferData.x, data.data.transferData.y);
  }
}

void Entity::getIfValue(const std::string& input, const std::string& key, int& value) const
{
  std::string fixedKey = key;

  if (input == "local")
  {
    fixedKey = getTag() + "@@" + fixedKey;
  }

  if (input == "local" || input == "global")
  {
    value = Persistent<int>::instance().get(fixedKey);
  }
  else if (input == "const")
  {
    if (fixedKey == "true" || fixedKey == "false")
    {
      value = fixedKey == "true";
    }
    else
    {
      value = atoi(key.c_str());
    }
  }
  else if (input == "item")
  {
    if (key != "gold")
    {
      Item* item = get_player()->getItem(replace_string(key, '_', ' '));
      if (item)
      {
        value = item->stackSize;
      }
      else
      {
        value = 0;
      }
    }
    else
    {
      value = get_player()->getGold();
    }
  }
  else
  {
    TRACE("Entity[%s]: Unknown input value %s from script.", getTag().c_str(), input.c_str());
  }
}

bool Entity::checkPlayerCollision() const
{
  if (m_walkThrough)
    return false;

  Entity* player = Game::instance().getPlayer()->player();

  if (player == this || player->m_walkThrough)
    return false;

  return (((int)player->x == x && (int)player->y == y) ||
          (player->getTargetX() == getTargetX() && player->getTargetY() == getTargetY()));
}

bool Entity::checkEntityCollision() const
{
  if (m_walkThrough)
    return false;

  Map* map = Game::instance().getCurrentMap();

  for (auto it = map->getEntities().begin(); it != map->getEntities().end(); ++it)
  {
    if ((*it) == this || (*it)->m_walkThrough)
      continue;

    int px = (*it)->x;
    int py = (*it)->y;

    if (px == x && py == y)
      return true;

    if (px == getTargetX() && py == getTargetY())
      return true;

    if (getTargetX() == (*it)->getTargetX() && getTargetY() == (*it)->getTargetY())
      return true;
  }

  return false;
}

std::string Entity::xmlDump() const
{
  std::ostringstream xml;

  xml << "<entity name=\"" << m_name << "\" tag=\"" << getTag() << "\">\n";

  xml << " <sprite name=\"" << m_sprite->getTextureName() << "\" />\n";
  xml << " <direction>" << directionToString(m_direction) << "</direction>\n";
  xml << " <speed>" << m_speed << "</speed>\n";
  xml << " <walkThrough>" << m_walkThrough << "</walkThrough>\n";
  xml << " <x>" << (int)x << "</x>\n";
  xml << " <y>" << (int)y << "</y>\n";

  xml << "</entity>\n";

  return xml.str();
}
