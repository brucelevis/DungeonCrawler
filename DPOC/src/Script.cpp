#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <stdexcept>

#include "Entity.h"
#include "Player.h"
#include "Game.h"
#include "SceneManager.h"
#include "Message.h"
#include "Persistent.h"
#include "Sound.h"
#include "Encounter.h"

#include "logger.h"
#include "Utility.h"
#include "Script.h"

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

static void get_if_value(const Entity* entity, const std::string& input, const std::string& key, int& value)
{
  std::string fixedKey = key;

  if (input == "local" && entity)
  {
    fixedKey = entity->getTag() + "@@" + fixedKey;
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
    TRACE("Unknown input value %s from script.", input.c_str());
  }
}

static bool is_comment(const std::string& str)
{
  for (size_t i = 0; i < str.size(); i++)
  {
    if (isspace(str[i]))
      continue;

    if (str[i] == '#')
    {
      return true;
    }
    else if (str[i] != '#')
    {
      return false;
    }
  }

  return false;
}

static bool all_whitespace(const std::string& str)
{
  for (size_t i = 0; i < str.size(); i++)
  {
    if (!isspace(str[i]))
    {
      return false;
    }
  }

  return true;
}

static void strip_comments(std::vector<std::string>& lines)
{
  for (auto it = lines.begin(); it != lines.end();)
  {
    if (it->size() == 0 || is_comment(*it) || all_whitespace(*it))
    {
      it = lines.erase(it);
    }
    else
    {
      ++it;
    }
  }
}

static std::string get_value_to_bracket(const std::string& str)
{
  std::string buffer;

  for (size_t i = 0; i < str.size(); i++)
  {
    if (str[i] == '[')
    {
      break;
    }
    else
    {
      buffer += str[i];
    }
  }

  return buffer;
}

static std::string get_value_in_bracket(const std::string& str)
{
  bool parsingBracket = false;
  std::string buffer;

  for (size_t i = 0; i < str.size(); i++)
  {
    if (str[i] == '[')
    {
      parsingBracket = true;
    }
    else if (str[i] == ']')
    {
      parsingBracket = false;
    }
    else if (parsingBracket)
    {
      buffer += str[i];
    }
  }

  return buffer;
}

static void get_if_statement_input(const std::string& data, std::string& key, std::string& type)
{
  if (data[0] == '$' || data[0] == '%' || isdigit(data[0]) || data == "true" || data == "false")
  {
    key = data;

    if (data[0] == '$')
    {
      type = "global";
    }
    else if (data[0] == '%')
    {
      type = "local";
    }
    else if (isdigit(data[0]) || data == "true" || data == "false")
    {
      type = "const";
    }
  }
  else
  {
    type = get_value_to_bracket(data);
    if (data == "item")
    {
      key = get_value_in_bracket(data);
    }
  }
}

static Script::ArithmOp get_arithm_op(const std::string& str)
{
  if (str == "+=") return Script::ARITHM_OP_ADD;
  if (str == "-=") return Script::ARITHM_OP_SUB;
  if (str == "*=") return Script::ARITHM_OP_MUL;
  if (str == "/=") return Script::ARITHM_OP_DIV;

  return Script::ARITHM_OP_UNKNOWN;
}

Script::Script()
 : m_currentIndex(0),
   m_running(false),
   m_loaded(false),
   m_callingEntity(0)
{

}

bool Script::loadFromFile(const std::string& file)
{
  m_currentIndex = 0;
  m_loaded = true;

  TRACE("Loading script %s", file.c_str());

  std::ifstream infile(file.c_str());
  if (infile.is_open())
  {
    std::vector<std::string> lines = get_lines(infile);

    loadFromLines(lines);

    infile.close();

    return true;
  }
  else
  {
    TRACE("Unable to open %s", file.c_str());
  }

  return false;
}

void Script::loadFromLines(std::vector<std::string> lines)
{
  strip_comments(lines);

  for (auto it = lines.begin(); it != lines.end(); ++it)
  {
    TRACE("Current Line = %s", it->c_str());
    ScriptData data = parseLine(*it);
    m_data.push_back(data);
  }

  m_loaded = true;
}

void Script::execute()
{
  m_currentIndex = 0;
  m_running = true;

  next();
}

void Script::next()
{
  executeScriptLine();
  advance();
}

void Script::advance()
{
  if (active())
  {
    m_currentIndex++;

    if (m_currentIndex >= m_data.size())
    {
      m_running = false;
    }
  }
}

void Script::stepBack()
{
  if (active())
  {
    m_currentIndex--;
    if (m_currentIndex < 0)
    {
      m_currentIndex = 0;
    }
  }
}

bool Script::active() const
{
  return m_running;
}

Script::ScriptData Script::getCurrentData() const
{
  return m_data[m_currentIndex];
}

bool Script::peekNext(ScriptData& out) const
{
  if (m_currentIndex < (m_data.size() - 1))
  {
    out = m_data[m_currentIndex + 1];

    return true;
  }

  return false;
}

Script::ScriptData Script::parseLine(const std::string& line) const
{
  std::vector<std::string> strings = split_string(line, ' ');

  Opcode opcode;

  if (strings[0][0] == '$' || strings[0][0] == '%')
  {
    // Variable operations. $ == global, % == local
    if (strings.size() <= 2)
    {
      TRACE("ERROR: Parse error for line: %s", line.c_str());
      throw std::runtime_error("Parse error for line: " + line);
    }

    if (strings[1] == "=")
    {
      opcode = OP_ASSIGNMENT;
    }
    else if (strings[1] == "+=" ||
             strings[1] == "-=" ||
             strings[1] == "*=" ||
             strings[1] == "/=")
    {
      opcode = OP_ARITHMETIC;
    }
  }
  else
  {
    // "Function call"
    opcode = getOpCode(strings[0]);
  }

  ScriptData data;
  data.opcode = opcode;

  if (opcode == OP_MESSAGE)
  {
    memset(data.data.messageData.message, 0, MAX_SCRIPT_MESSAGE_BUFFER_SIZE);

    for (size_t i = 1; i < strings.size(); i++)
    {
      std::strcat(data.data.messageData.message, strings[i].c_str());
      if (i < strings.size() - 1)
      {
        std::strcat(data.data.messageData.message, " ");
      }
    }
  }
  else if (opcode == OP_WALK)
  {
    data.data.walkData.dir = directionFromString(strings[1]);
  }
  else if (opcode == OP_SET_DIR)
  {
    data.data.walkData.dir = directionFromString(strings[1]);
  }
  else if (opcode == OP_WAIT)
  {
    data.data.waitData.duration = atoi(strings[1].c_str());
  }
  else if (opcode == OP_ASSIGNMENT || opcode == OP_ARITHMETIC)
  {
    std::string key = strings[0];
    std::string value = strings[2];

    std::string assign_key;
    std::string assign_type;

    get_if_statement_input(value, assign_key, assign_type);

    if (opcode == OP_ASSIGNMENT)
    {
      memset(data.data.setPersistentData.key, 0, MAX_SCRIPT_KEY_SIZE);
      memset(data.data.setPersistentData.type, 0, MAX_SCRIPT_KEY_SIZE);
      memset(data.data.setPersistentData.value, 0, MAX_SCRIPT_KEY_SIZE);

      strcpy(data.data.setPersistentData.key, key.c_str());
      strcpy(data.data.setPersistentData.type, assign_type.c_str());
      strcpy(data.data.setPersistentData.value, assign_key.c_str());
    }
    else if (opcode == OP_ARITHMETIC)
    {
      memset(data.data.arithmeticData.key, 0, MAX_SCRIPT_KEY_SIZE);
      memset(data.data.arithmeticData.type, 0, MAX_SCRIPT_KEY_SIZE);
      memset(data.data.arithmeticData.value, 0, MAX_SCRIPT_KEY_SIZE);

      data.data.arithmeticData.operation = get_arithm_op(strings[1]);
      strcpy(data.data.arithmeticData.key, key.c_str());
      strcpy(data.data.arithmeticData.type, assign_type.c_str());
      strcpy(data.data.arithmeticData.value, assign_key.c_str());
    }
  }
  else if (opcode == OP_IF)
  {
    std::string lhs = strings[1];
    std::string operation = strings[2];
    std::string rhs = strings[3];

    std::string lhs_what, rhs_what;
    std::string lhs_key, rhs_key;

    memset(data.data.ifData.rhs, 0, MAX_SCRIPT_KEY_SIZE);
    memset(data.data.ifData.lhs, 0, MAX_SCRIPT_KEY_SIZE);
    memset(data.data.ifData.rhsKey, 0, MAX_SCRIPT_KEY_SIZE);
    memset(data.data.ifData.lhsKey, 0, MAX_SCRIPT_KEY_SIZE);
    memset(data.data.ifData.boolOperation, 0, MAX_SCRIPT_KEY_SIZE);

    get_if_statement_input(lhs, lhs_key, lhs_what);
    get_if_statement_input(rhs, rhs_key, rhs_what);

    TRACE("OP_IF: (%s %s %s), lhs_what=%s, rhs_what=%s, lhs_key=%s, rhs_key=%s, operation='%s'",
        lhs.c_str(), operation.c_str(), rhs.c_str(), lhs_what.c_str(), rhs_what.c_str(), lhs_key.c_str(), rhs_key.c_str(), operation.c_str());

    strcpy(data.data.ifData.rhs, rhs_what.c_str());
    strcpy(data.data.ifData.lhs, lhs_what.c_str());
    strcpy(data.data.ifData.rhsKey, rhs_key.c_str());
    strcpy(data.data.ifData.lhsKey, lhs_key.c_str());
    strcpy(data.data.ifData.boolOperation, operation.c_str());
  }
  else if (opcode == OP_END_IF)
  {
    // Nothing
  }
  else if (opcode == OP_ELSE)
  {
    // Nothing
  }
  else if (opcode == OP_CHOICE)
  {
    for (int i = 0; i < MAX_CHOICES; i++)
    {
      memset(data.data.choiceData.choices[i], '\0', MAX_SCRIPT_KEY_SIZE);
    }

    std::string all;
    for (size_t i = 1; i < strings.size(); i++)
    {
      all += strings[i];
      if (i < strings.size() - 1)
        all += " ";
    }

    std::vector<std::string> choices = split_string(all, ',');

    data.data.choiceData.numberOfChoices = choices.size();
    for (size_t i = 0; i < choices.size(); i++)
    {
      strcpy(data.data.choiceData.choices[i], choices[i].c_str());
    }
  }
  else if (opcode == OP_SET_TILE_ID)
  {
    data.data.setTileIdData.tileId = atoi(strings[1].c_str());
  }
  else if (opcode == OP_GIVE_ITEM || opcode == OP_TAKE_ITEM)
  {
    memset(data.data.giveItemData.itemName, '\0', MAX_SCRIPT_KEY_SIZE);
    data.data.giveItemData.amount = atoi(strings[1].c_str());
    for (size_t i = 2; i < strings.size(); i++)
    {
      strcat(data.data.giveItemData.itemName, strings[i].c_str());
      if (i < strings.size() - 1)
        strcat(data.data.giveItemData.itemName, " ");
    }
  }
  else if (opcode == OP_GIVE_GOLD || opcode == OP_TAKE_GOLD)
  {
    data.data.giveGoldData.amount = atoi(strings[1].c_str());
  }
  else if (opcode == OP_PLAY_SOUND)
  {
    memset(data.data.playSoundData.sound, '\0', MAX_SCRIPT_KEY_SIZE);
    strcpy(data.data.playSoundData.sound, strings[1].c_str());
  }
  else if (opcode == OP_ADD_PARTY_MEMBER)
  {
    memset(data.data.addPartyMemberData.className, '\0', MAX_SCRIPT_KEY_SIZE);
    memset(data.data.addPartyMemberData.name, '\0', MAX_SCRIPT_KEY_SIZE);
    data.data.addPartyMemberData.level = 1;

    strcpy(data.data.addPartyMemberData.name, strings[1].c_str());
    strcpy(data.data.addPartyMemberData.className, strings[2].c_str());
    data.data.addPartyMemberData.level = atoi(strings[3].c_str());
  }
  else if (opcode == OP_REMOVE_PARTY_MEMBER)
  {
    memset(data.data.removePartyMemberData.name, '\0', MAX_SCRIPT_KEY_SIZE);
    strcpy(data.data.removePartyMemberData.name, strings[1].c_str());
  }
  else if (opcode == OP_SET_VISIBLE)
  {
    if (strings[1] == "true")
      data.data.setVisibleData.visibility = true;
    else
      data.data.setVisibleData.visibility = false;
  }
  else if (opcode == OP_SET_WALKTHROUGH)
  {
    if (strings[1] == "true")
      data.data.setWalkthroughData.walkthrough = true;
    else
      data.data.setWalkthroughData.walkthrough = false;
  }
  else if (opcode == OP_ENABLE_CONTROLS)
  {
    if (strings[1] == "true")
      data.data.enableControlsData.enabled = true;
    else
      data.data.enableControlsData.enabled = false;
  }
  else if (opcode == OP_RECOVER_ALL)
  {
    // Nothing
  }
  else if (opcode == OP_COMBAT || opcode == OP_COMBAT_NO_ESAPE)
  {
    memset(data.data.combatData.monsters, '\0', MAX_SCRIPT_KEY_SIZE * MAX_SCRIPT_KEY_SIZE);

    data.data.combatData.canEscape = opcode != OP_COMBAT_NO_ESAPE;

    std::string all;

    for (size_t i = 1; i < strings.size(); i++)
    {
      all += strings[i];
      if (i < strings.size() - 1)
        all += " ";
    }

    // Error checking
    if (all.size() >= MAX_SCRIPT_KEY_SIZE * MAX_SCRIPT_KEY_SIZE)
    {
      TRACE("Too long string for combat command: %s", all.c_str());
      throw std::runtime_error("Too long string for combat command!");
    }

    std::vector<std::string> monsters = split_string(all, ',');

    for (size_t i = 0; i < monsters.size(); i++)
    {
      strcpy(data.data.combatData.monsters[i], monsters[i].c_str());
    }

    data.data.combatData.number = monsters.size();
  }
  else if (opcode == OP_ENCOUNTER)
  {
    memset(data.data.encounterData.encounterName, '\0', MAX_SCRIPT_KEY_SIZE);
    std::string buffer;
    for (size_t i = 1; i < strings.size(); i++)
    {
      buffer += strings[i];
      if (i < strings.size() - 1)
        buffer += " ";
    }

    strcpy(data.data.encounterData.encounterName, buffer.c_str());
  }
  else if (opcode == OP_END_GAME)
  {
    // Nothing
  }
  else if (opcode == OP_SET_CONFIG)
  {
    memset(data.data.setConfigData.key, '\0', MAX_SCRIPT_KEY_SIZE);
    memset(data.data.setConfigData.value, '\0', MAX_SCRIPT_KEY_SIZE);

    strcpy(data.data.setConfigData.key, strings[1].c_str());
    strcpy(data.data.setConfigData.value, strings[2].c_str());
  }
  else if (opcode == OP_TRANSFER)
  {
    memset(data.data.transferData.targetMap, '\0', MAX_SCRIPT_KEY_SIZE);
    strcpy(data.data.transferData.targetMap, strings[1].c_str());

    data.data.transferData.x = atoi(strings[2].c_str());
    data.data.transferData.y = atoi(strings[3].c_str());
  }
  else if (opcode == OP_SHOP)
  {
    memset(data.data.combatData.monsters, '\0', MAX_SCRIPT_KEY_SIZE * 32);

    std::string all;

    for (size_t i = 1; i < strings.size(); i++)
    {
      all += strings[i];
      if (i < strings.size() - 1)
        all += " ";
    }

    // Error checking
    if (all.size() >= MAX_SCRIPT_KEY_SIZE * 32)
    {
      TRACE("Too long string for shop command: %s", all.c_str());
      throw std::runtime_error("Too long string for shop command!");
    }

    std::vector<std::string> items = split_string(all, ',');

    for (size_t i = 0; i < items.size(); i++)
    {
      strcpy(data.data.shopData.inventory[i], items[i].c_str());
    }

    data.data.shopData.number = items.size();
  }
  else if (opcode == OP_SHOW_PICTURE)
  {
    memset(data.data.showPictureData.name, '\0', MAX_SCRIPT_KEY_SIZE);
    strcpy(data.data.showPictureData.name, strings[1].c_str());
    data.data.showPictureData.x = atoi(strings[2].c_str());
    data.data.showPictureData.y = atoi(strings[3].c_str());
  }
  else if (opcode == OP_HIDE_PICTURE)
  {
    memset(data.data.hidePictureData.name, '\0', MAX_SCRIPT_KEY_SIZE);
    strcpy(data.data.hidePictureData.name, strings[1].c_str());
  }
  else
  {
    TRACE("Error when parsing line %s: No matching opcode found.", line.c_str());
  }

  return data;
}

Script::Opcode Script::getOpCode(const std::string& opStr) const
{
  static std::map<std::string, Opcode> OP_MAP =
  {
    { "message",      OP_MESSAGE },
    { "walk",         OP_WALK },
    { "set_dir",      OP_SET_DIR },
    { "wait",         OP_WAIT },
//    { "set_global",   OP_SET_GLOBAL },
//    { "set_local",    OP_SET_LOCAL },
    { "if",           OP_IF },
    { "endif",        OP_END_IF },
    { "else",         OP_ELSE },
    { "choice",       OP_CHOICE },
    { "set_tile_id",  OP_SET_TILE_ID },
    { "give_item",    OP_GIVE_ITEM },
    { "take_item",    OP_TAKE_ITEM },
    { "give_gold",    OP_GIVE_GOLD },
    { "take_gold",    OP_TAKE_GOLD },
    { "play_sound",   OP_PLAY_SOUND },
    { "add_member",   OP_ADD_PARTY_MEMBER },
    { "remove_member", OP_REMOVE_PARTY_MEMBER },
    { "set_visible",  OP_SET_VISIBLE },
    { "set_walkthrough", OP_SET_WALKTHROUGH },
    { "enable_controls", OP_ENABLE_CONTROLS },
    { "recover_all",  OP_RECOVER_ALL },
    { "combat",       OP_COMBAT },
    { "combat_no_escape", OP_COMBAT_NO_ESAPE },
    { "encounter",    OP_ENCOUNTER },
    { "end_game",     OP_END_GAME },
    { "set_config",   OP_SET_CONFIG },
    { "transfer",     OP_TRANSFER },
    { "shop",         OP_SHOP },
    { "show_picture", OP_SHOW_PICTURE },
    { "hide_picture", OP_HIDE_PICTURE }
  };

  auto it = OP_MAP.find(opStr);
  if (it != OP_MAP.end())
  {
    return it->second;
  }

  TRACE("UNKNOWN OPCODE %s!!!", opStr.c_str());
  throw std::runtime_error("UNKNOWN OPCODE " + opStr + "!!!");

  return OP_NOP;
}

void Script::setCallingEntity(Entity* entity)
{
  m_callingEntity = entity;
}

void Script::executeScriptLine()
{
  const Script::ScriptData& data = getCurrentData();

  if (data.opcode == Script::OP_MESSAGE)
  {
    Message::instance().show(data.data.messageData.message);

    Script::ScriptData nextLine;
    if (peekNext(nextLine))
    {
      if (nextLine.opcode == Script::OP_MESSAGE || nextLine.opcode == Script::OP_CHOICE)
      {
        advance();
        executeScriptLine();
      }
    }
  }
  else if (data.opcode == Script::OP_WALK)
  {
    if (m_callingEntity)
    {
      m_callingEntity->step(data.data.walkData.dir);
    }
  }
  else if (data.opcode == Script::OP_SET_DIR)
  {
    if (m_callingEntity)
    {
      // Script should override fixed direction property.
      bool fixTemp = m_callingEntity->m_fixedDirection;
      m_callingEntity->setFixedDirection(false);

      m_callingEntity->setDirection(data.data.walkData.dir);

      m_callingEntity->setFixedDirection(fixTemp);
    }
  }
  else if (data.opcode == Script::OP_WAIT)
  {
    if (m_callingEntity)
    {
      m_callingEntity->m_scriptWaitMap[this] = data.data.waitData.duration;
    }
  }
  else if (data.opcode == Script::OP_ASSIGNMENT)
  {
    std::string key = data.data.setPersistentData.key;
    std::string assign_key = data.data.setPersistentData.value;
    std::string assign_type = data.data.setPersistentData.type;

    int value;

    get_if_value(m_callingEntity, assign_type, assign_key, value);

    if (key[0] == '$')
    {
      Persistent<int>::instance().set(key, value);
    }
    else if (key[0] == '%')
    {
      Persistent<int>::instance().set(m_callingEntity->getTag() + "@@" + key, value);
    }

    advance();
    executeScriptLine();
  }
  else if (data.opcode == Script::OP_ARITHMETIC)
  {
    std::string key = data.data.arithmeticData.key;
    std::string assign_key = data.data.arithmeticData.value;
    std::string assign_type = data.data.arithmeticData.type;
    ArithmOp operation = data.data.arithmeticData.operation;

    int value;

    get_if_value(m_callingEntity, assign_type, assign_key, value);

    if (key[0] == '$')
    {
      int current = Persistent<int>::instance().get(key);

      if (operation == ARITHM_OP_ADD) current += value;
      if (operation == ARITHM_OP_SUB) current -= value;
      if (operation == ARITHM_OP_MUL) current *= value;
      if (operation == ARITHM_OP_DIV) current /= value;

      Persistent<int>::instance().set(key, current);
    }
    else if (key[0] == '%')
    {
      int current = Persistent<int>::instance().get(m_callingEntity->getTag() + "@@" + key);

      if (operation == ARITHM_OP_ADD) current += value;
      if (operation == ARITHM_OP_SUB) current -= value;
      if (operation == ARITHM_OP_MUL) current *= value;
      if (operation == ARITHM_OP_DIV) current /= value;

      Persistent<int>::instance().set(m_callingEntity->getTag() + "@@" + key, current);
    }

    advance();
    executeScriptLine();
  }
  else if (data.opcode == Script::OP_IF)
  {
    std::string lhs = data.data.ifData.lhs;
    std::string rhs = data.data.ifData.rhs;
    std::string lhsKey = data.data.ifData.lhsKey;
    std::string rhsKey = data.data.ifData.rhsKey;
    std::string operation = data.data.ifData.boolOperation;

    int lhsValue, rhsValue;

    get_if_value(m_callingEntity, lhs, lhsKey, lhsValue);
    get_if_value(m_callingEntity, rhs, rhsKey, rhsValue);

    bool result = exec_bool_operation(operation, lhsValue, rhsValue);

    if (!result)
    {
      // Find matching end_if or else, in case the expression was false.
      int ifCount = 1;
      while (ifCount > 0)
      {
        advance();
        if (getCurrentData().opcode == Script::OP_IF)
        {
          ifCount++;
        }
        else if (getCurrentData().opcode == Script::OP_END_IF)
        {
          ifCount--;
        }
        else if (getCurrentData().opcode == Script::OP_ELSE && ifCount == 1)
        {
          // IfCount == 1 means it is the original IF, so enter this ELSE branch.
          ifCount--;
        }
      }
    }

    advance();
    executeScriptLine();
  }
  else if (data.opcode == Script::OP_ELSE)
  {
    // In case we advanced into an ELSE opcode, continue until we find the matching END.
    int ifCount = 1;
    while (ifCount > 0)
    {
      advance();
      if (getCurrentData().opcode == Script::OP_IF)
      {
        ifCount++;
      }
      else if (getCurrentData().opcode == Script::OP_END_IF)
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
    if (m_callingEntity)
    {
      int tileId = data.data.setTileIdData.tileId;

      TileSprite* tileSprite = dynamic_cast<TileSprite*>(m_callingEntity->m_sprite);
      if (tileSprite)
      {
        tileSprite->setTileNum(tileId);
      }
      else
      {
        TRACE("Attempting to call set_tile_id on entity not using tileSprite.");
      }
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
    play_sound("Audio/" + sound);
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
    if (m_callingEntity)
    {
      m_callingEntity->m_visible = data.data.setVisibleData.visibility;
    }
  }
  else if (data.opcode == Script::OP_SET_WALKTHROUGH)
  {
    if (m_callingEntity)
    {
      m_callingEntity->m_walkThrough = data.data.setWalkthroughData.walkthrough;
    }
  }
  else if (data.opcode == Script::OP_ENABLE_CONTROLS)
  {
    get_player()->setControlsEnabled(data.data.enableControlsData.enabled);
  }
  else if (data.opcode == Script::OP_RECOVER_ALL)
  {
    get_player()->recoverAll();
  }
  else if (data.opcode == Script::OP_COMBAT || data.opcode == Script::OP_COMBAT_NO_ESAPE)
  {
    std::vector<std::string> monsters;

    for (int i = 0; i < data.data.combatData.number; i++)
    {
      monsters.push_back(data.data.combatData.monsters[i]);
    }

    Game::instance().startBattle(monsters, data.data.combatData.canEscape);
  }
  else if (data.opcode == Script::OP_ENCOUNTER)
  {
    std::string encounterName = data.data.encounterData.encounterName;

    const Encounter* encounter = get_encounter(encounterName);
    if (encounter)
    {
      encounter->start();
    }
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
    else if (data.opcode == Script::OP_SHOP)
    {
      std::vector<std::string> items;

      for (int i = 0; i < data.data.shopData.number; i++)
      {
        items.push_back(data.data.shopData.inventory[i]);
      }

      Game::instance().openShop(items);
    }
    else if (data.opcode == Script::OP_SHOW_PICTURE)
    {
      std::string name = data.data.showPictureData.name;
      float x = data.data.showPictureData.x;
      float y = data.data.showPictureData.y;

      SceneManager::instance().showPicture(name, x, y);
    }
    else if (data.opcode == Script::OP_HIDE_PICTURE)
    {
      std::string name = data.data.hidePictureData.name;

      SceneManager::instance().hidePicture(name);
    }
}

