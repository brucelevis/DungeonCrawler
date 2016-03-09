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
#include "Battle.h"

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
    value = Persistent::instance().getAs<int>(fixedKey);
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

static void replace_arguments(std::vector<std::string>& lines, const std::vector<std::string>& arguments)
{
  for (std::string& line : lines)
  {
    for (size_t i = 0; i < arguments.size(); i++)
    {
      int varIndex = i + 1;
      std::string varToken = "@" + toString(varIndex);

      size_t pos = std::string::npos;
      while ((pos = line.find(varToken)) != std::string::npos)
      {
        line.replace(pos, varToken.size(), arguments[i]);
      }
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
    if (type == "item")
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

static std::string replace_variables_in_string(const std::string& str, const Entity* callingEntity)
{
  std::string buffer = str;

  std::vector<std::string> variables;

  bool parseVar = false;
  std::string variable;
  for (size_t i = 0; i < str.size(); i++)
  {
    if (!parseVar)
    {
      if (str[i] == '$' || (str[i] == '%' && callingEntity))
      {
        parseVar = true;
        variable += str[i];
      }
    }
    else
    {
      if ((str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= 'a' && str[i] <= 'z') || isdigit(str[i]) || str[i] == '_' || str[i] == ':')
      {
        variable += str[i];

        if (i == str.size() - 1)
        {
          variables.push_back(variable);
        }
      }
      else
      {
        parseVar = false;
        variables.push_back(variable);
        variable = "";

        // if we have something like: %var1$var2
        if (i > 0) i--;
      }
    }
  }

  for (auto it = variables.begin(); it != variables.end(); ++it)
  {
    const std::string& variable = *it;
    std::string key = variable;

    if (variable[0] == '%')
    {
      key = callingEntity->getTag() + "@@" + variable;
    }

    if (Persistent::instance().isSet(key))
    {
      std::string value = Persistent::instance().get(key);

      size_t pos = buffer.find(variable);
      buffer.replace(pos, variable.size(), value);
    }
  }

  return buffer;
}

Script::Script()
 : m_currentIndex(0),
   m_running(false),
   m_loaded(false),
   m_callingEntity(nullptr),
   m_callingBattle(nullptr)
{

}

bool Script::loadFromFile(const std::string& file, const std::vector<std::string>& arguments)
{
  m_currentIndex = 0;
  m_loaded = true;

  TRACE("Loading script %s", file.c_str());

  std::ifstream infile(file.c_str());
  if (infile.is_open())
  {
    std::vector<std::string> lines = get_lines(infile);

    loadFromLines(lines, arguments);

    infile.close();

    return true;
  }
  else
  {
    TRACE("Unable to open %s", file.c_str());
  }

  return false;
}

void Script::loadFromLines(std::vector<std::string> lines, const std::vector<std::string>& arguments)
{
  strip_comments(lines);
  replace_arguments(lines, arguments);

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

bool Script::active() const
{
  return m_running;
}

const Script::ScriptData& Script::getCurrentData() const
{
  static ScriptData dummy = { OP_NOP };
  if (m_currentIndex >= m_data.size())
    return dummy;

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

  Opcode opcode = OP_NOP;

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
    std::string message;

    for (size_t i = 1; i < strings.size(); i++)
    {
      message.append(strings[i]);

      if (i < strings.size() - 1)
      {
        message.push_back(' ');
      }
    }

    data.arguments["message"] = message;
  }
  else if (opcode == OP_WALK)
  {
    data.arguments["direction"] = strings[1];
  }
  else if (opcode == OP_SET_DIR || opcode == OP_SET_PLAYER_DIR)
  {
    data.arguments["direction"] = strings[1];
  }
  else if (opcode == OP_WAIT)
  {
    data.arguments["duration"] = strings[1];
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
      data.arguments["key"] = key;
      data.arguments["type"] = assign_type;
      data.arguments["value"] = assign_key;
    }
    else if (opcode == OP_ARITHMETIC)
    {
      data.arguments["operation"] = strings[1];
      data.arguments["key"] = key;
      data.arguments["type"] = assign_type;
      data.arguments["value"] = assign_key;
    }
  }
  else if (opcode == OP_IF || opcode == OP_WHILE)
  {
    std::string lhs = strings[1];
    std::string operation = strings[2];
    std::string rhs = strings[3];

    std::string lhs_what, rhs_what;
    std::string lhs_key, rhs_key;

    get_if_statement_input(lhs, lhs_key, lhs_what);
    get_if_statement_input(rhs, rhs_key, rhs_what);

    TRACE("OP_IF: (%s %s %s), lhs_what=%s, rhs_what=%s, lhs_key=%s, rhs_key=%s, operation='%s'",
        lhs.c_str(), operation.c_str(), rhs.c_str(), lhs_what.c_str(), rhs_what.c_str(), lhs_key.c_str(), rhs_key.c_str(), operation.c_str());

    data.arguments["rhs"] = rhs_what;
    data.arguments["lhs"] = lhs_what;
    data.arguments["rhsKey"] = rhs_key;
    data.arguments["lhsKey"] = lhs_key;
    data.arguments["boolOperation"] = operation;
  }
  else if (opcode == OP_END_IF)
  {
    // Nothing
  }
  else if (opcode == OP_ELSE)
  {
    // Nothing
  }
  else if (opcode == OP_WEND)
  {
    // Nothing
  }
  else if (opcode == OP_BREAK)
  {
    // Nothing
  }
  else if (opcode == OP_CHOICE)
  {
    std::string all;
    for (size_t i = 1; i < strings.size(); i++)
    {
      all += strings[i];
      if (i < strings.size() - 1)
        all += " ";
    }

    std::vector<std::string> choices = split_string(all, ',');

    for (size_t i = 0; i < choices.size(); i++)
    {
      data.listArguments["choices"].push_back(choices[i]);
    }
  }
  else if (opcode == OP_SET_TILE_ID)
  {
    data.arguments["tileId"] = strings[1];
  }
  else if (opcode == OP_GIVE_ITEM || opcode == OP_TAKE_ITEM)
  {
    data.arguments["amount"] = strings[1];

    std::string itemName;

    for (size_t i = 2; i < strings.size(); i++)
    {
      itemName.append(strings[i]);
      if (i < strings.size() - 1)
      {
        itemName.push_back(' ');
      }
    }

    data.arguments["itemName"] = itemName;
  }
  else if (opcode == OP_GIVE_GOLD || opcode == OP_TAKE_GOLD)
  {
    data.arguments["amount"] = strings[1];
  }
  else if (opcode == OP_PLAY_SOUND)
  {
    data.arguments["sound"] = strings[1];
  }
  else if (opcode == OP_ADD_PARTY_MEMBER)
  {
    data.arguments["name"] = strings[1];
    data.arguments["className"] = strings[2];
    data.arguments["level"] = strings[3];
  }
  else if (opcode == OP_REMOVE_PARTY_MEMBER)
  {
    data.arguments["name"] = strings[1];
  }
  else if (opcode == OP_SET_VISIBLE)
  {
    data.arguments["visibility"] = strings[1];
  }
  else if (opcode == OP_SET_WALKTHROUGH)
  {
    data.arguments["walkthrough"] = strings[1];
  }
  else if (opcode == OP_ENABLE_CONTROLS)
  {
    data.arguments["enabled"] = strings[1];
  }
  else if (opcode == OP_RECOVER_ALL)
  {
    // Nothing
  }
  else if (opcode == OP_COMBAT || opcode == OP_COMBAT_NO_ESAPE)
  {
    data.arguments["canEscape"] = opcode != OP_COMBAT_NO_ESAPE ? "true" : "false";

    std::string all;

    for (size_t i = 1; i < strings.size(); i++)
    {
      all += strings[i];
      if (i < strings.size() - 1)
        all += " ";
    }

    std::vector<std::string> monsters = split_string(all, ',');

    for (size_t i = 0; i < monsters.size(); i++)
    {
      data.listArguments["monsters"].push_back(monsters[i]);
    }
  }
  else if (opcode == OP_ENCOUNTER)
  {
    std::string buffer;
    for (size_t i = 1; i < strings.size(); i++)
    {
      buffer += strings[i];
      if (i < strings.size() - 1)
        buffer += " ";
    }

    data.arguments["encounterName"] = buffer;
  }
  else if (opcode == OP_END_GAME)
  {
    // Nothing
  }
  else if (opcode == OP_SET_CONFIG)
  {
    data.arguments["key"] = strings[1];
    data.arguments["value"] = strings[2];
  }
  else if (opcode == OP_TRANSFER)
  {
    data.arguments["targetMap"] = strings[1];
    data.arguments["x"] = strings[2];
    data.arguments["y"] = strings[3];
    data.arguments["dir"] = strings.size() > 4 ? strings[4] : "DIR_RANDOM";
    // If a direction is given update player dir after transfer.
  }
  else if (opcode == OP_SHOP)
  {
    std::string all;

    for (size_t i = 1; i < strings.size(); i++)
    {
      all += strings[i];
      if (i < strings.size() - 1)
        all += " ";
    }

    std::vector<std::string> items = split_string(all, ',');

    for (size_t i = 0; i < items.size(); i++)
    {
      data.listArguments["inventory"].push_back(items[i]);
    }
  }
  else if (opcode == OP_SHOW_PICTURE)
  {
    data.arguments["name"] = strings[1];
    data.arguments["x"] = strings[2];
    data.arguments["y]"] = strings[3];
  }
  else if (opcode == OP_HIDE_PICTURE)
  {
    data.arguments["name"] = strings[1];
  }
  else if (opcode == OP_SKILL_TRAINER)
  {
    std::string all;

    for (size_t i = 1; i < strings.size(); i++)
    {
      all += strings[i];
      if (i < strings.size() - 1)
        all += " ";
    }

    std::vector<std::string> skills = split_string(all, ',');

    for (size_t i = 0; i < skills.size(); i++)
    {
      data.listArguments["skills"].push_back(skills[i]);
    }
  }
  else if (opcode == OP_CAMPSITE)
  {

  }
  else if (opcode == OP_CHANGE_TILE)
  {
    data.arguments["layer"] = strings[1];
    data.arguments["x"] = strings[2];
    data.arguments["y"] = strings[3];
    data.arguments["tilenum"] = strings[4];
  }
  else if (opcode == OP_FLASH_SCREEN)
  {
    data.arguments["duration"] = strings[1];
    data.arguments["r"] = strings[2];
    data.arguments["g"] = strings[3];
    data.arguments["b"] = strings[4];
  }
  else if (opcode == OP_CHANGE_PLAYER_POSITION)
  {
    data.arguments["x"] = strings[1];
    data.arguments["y"] = strings[2];
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
    { "if",           OP_IF },
    { "endif",        OP_END_IF },
    { "else",         OP_ELSE },
    { "while",        OP_WHILE },
    { "wend",         OP_WEND },
    { "break",        OP_BREAK },
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
    { "hide_picture", OP_HIDE_PICTURE },
    { "skill_trainer", OP_SKILL_TRAINER },
    { "campsite", OP_CAMPSITE },
    { "set_player_dir", OP_SET_PLAYER_DIR },
    { "change_tile", OP_CHANGE_TILE },
    { "flash_screen", OP_FLASH_SCREEN },
    { "change_player_position", OP_CHANGE_PLAYER_POSITION }
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

void Script::setCallingBattle(Battle* battle)
{
  m_callingBattle = battle;
}

std::string Script::extractValue(const std::string& input) const
{
  if (input.size())
  {
    if (input[0] == '$')
    {
      if (Persistent::instance().isSet(input))
      {
        return Persistent::instance().get(input);
      }
    }
    else if (input[0] == '%' && m_callingEntity)
    {
      std::string key = m_callingEntity->getTag() + "@@" + input;

      if (Persistent::instance().isSet(key))
      {
        return Persistent::instance().get(key);
      }
    }
  }

  return input;
}

void Script::executeScriptLine()
{
  const Script::ScriptData& data = getCurrentData();

  if (data.opcode == Script::OP_MESSAGE)
  {
    std::string msg = replace_variables_in_string(data.arguments.at("message"), m_callingEntity);
    Message::instance().show(msg);

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
      m_callingEntity->step(directionFromString(extractValue(data.arguments.at("dir"))));
    }
  }
  else if (data.opcode == Script::OP_SET_DIR)
  {
    if (m_callingEntity)
    {
      // Script should override fixed direction property.
      bool fixTemp = m_callingEntity->m_fixedDirection;
      m_callingEntity->setFixedDirection(false);

      m_callingEntity->setDirection(directionFromString(extractValue(data.arguments.at("dir"))));

      m_callingEntity->setFixedDirection(fixTemp);
    }
  }
  else if (data.opcode == Script::OP_WAIT)
  {
    if (m_callingEntity)
    {
      m_callingEntity->m_scriptWaitMap[this] = fromString<int>(extractValue(data.arguments.at("duration")));
    }
    else if (m_callingBattle)
    {
      m_callingBattle->m_turnDelay = fromString<int>(extractValue(data.arguments.at("duration")));
    }
  }
  else if (data.opcode == Script::OP_ASSIGNMENT)
  {
    std::string key = data.arguments.at("key");
    std::string assign_key = data.arguments.at("value");
    std::string assign_type = data.arguments.at("type");

    int value;

    get_if_value(m_callingEntity, assign_type, assign_key, value);

    if (key[0] == '$')
    {
      Persistent::instance().set(key, value);
    }
    else if (key[0] == '%')
    {
      Persistent::instance().set(m_callingEntity->getTag() + "@@" + key, value);
    }

    advance();
    executeScriptLine();
  }
  else if (data.opcode == Script::OP_ARITHMETIC)
  {
    std::string key = data.arguments.at("key");
    std::string assign_key = data.arguments.at("value");
    std::string assign_type = data.arguments.at("type");
    ArithmOp operation = get_arithm_op(data.arguments.at("operation"));

    int value;

    get_if_value(m_callingEntity, assign_type, assign_key, value);

    if (key[0] == '$')
    {
      int current = Persistent::instance().getAs<int>(key);

      if (operation == ARITHM_OP_ADD) current += value;
      if (operation == ARITHM_OP_SUB) current -= value;
      if (operation == ARITHM_OP_MUL) current *= value;
      if (operation == ARITHM_OP_DIV) current /= value;

      Persistent::instance().set(key, current);
    }
    else if (key[0] == '%')
    {
      int current = Persistent::instance().getAs<int>(m_callingEntity->getTag() + "@@" + key);

      if (operation == ARITHM_OP_ADD) current += value;
      if (operation == ARITHM_OP_SUB) current -= value;
      if (operation == ARITHM_OP_MUL) current *= value;
      if (operation == ARITHM_OP_DIV) current /= value;

      Persistent::instance().set(m_callingEntity->getTag() + "@@" + key, current);
    }

    advance();
    executeScriptLine();
  }
  else if (data.opcode == Script::OP_IF)
  {
    std::string lhs = data.arguments.at("lhs");
    std::string rhs = data.arguments.at("rhs");
    std::string lhsKey = data.arguments.at("lhsKey");
    std::string rhsKey = data.arguments.at("rhsKey");
    std::string operation = data.arguments.at("boolOperation");

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
  else if (data.opcode == Script::OP_WHILE)
  {
    std::string lhs = data.arguments.at("lhs");
    std::string rhs = data.arguments.at("rhs");
    std::string lhsKey = data.arguments.at("lhsKey");
    std::string rhsKey = data.arguments.at("rhsKey");
    std::string operation = data.arguments.at("boolOperation");

    int lhsValue, rhsValue;

    get_if_value(m_callingEntity, lhs, lhsKey, lhsValue);
    get_if_value(m_callingEntity, rhs, rhsKey, rhsValue);

    bool result = exec_bool_operation(operation, lhsValue, rhsValue);

    if (!result)
    {
      // Find matching wend in case the expression was false.
      int whileCount = 1;
      while (whileCount > 0)
      {
        advance();
        if (getCurrentData().opcode == Script::OP_WHILE)
        {
          whileCount++;
        }
        else if (getCurrentData().opcode == Script::OP_WEND)
        {
          whileCount--;
        }
      }
    }

    advance();
    executeScriptLine();
  }
  else if (data.opcode == Script::OP_WEND)
  {
    // Step backward in script until finding matching WHILE

    int wendCount = 1;
    while (wendCount > 0)
    {
      if (m_currentIndex == 0)
      {
        throw std::runtime_error{"Badly form script, wend without matching while."};
      }

      m_currentIndex--;

      if (getCurrentData().opcode == Script::OP_WHILE)
      {
        wendCount--;
      }
      else if (getCurrentData().opcode == Script::OP_WEND)
      {
        wendCount++;
      }
    }

    // Execute the WHILE that we stand on.
    executeScriptLine();
  }
  else if (data.opcode == Script::OP_BREAK)
  {
    // Step until finding WEND
    while (getCurrentData().opcode != OP_WEND)
    {
      advance();
    }

    // Step past WEND and execute.
    advance();
    executeScriptLine();
  }
  else if (data.opcode == Script::OP_CHOICE)
  {
    std::vector<std::string> choices;

    for (const auto& choice : data.listArguments.at("choices"))
    {
      choices.push_back(replace_string(extractValue(choice), '_', ' '));
    }

    Game::instance().openChoiceMenu(choices);
  }
  else if (data.opcode == Script::OP_SET_TILE_ID)
  {
    if (m_callingEntity)
    {
      int tileId = fromString<int>(extractValue(data.arguments.at("tileId")));

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
    int amount = fromString<int>(extractValue(data.arguments.at("amount")));
    std::string itemName = data.arguments.at(extractValue("itemName"));

    get_player()->addItemToInventory(itemName, amount);
  }
  else if (data.opcode == Script::OP_TAKE_ITEM)
  {
    int amount = fromString<int>(extractValue(data.arguments.at("amount")));
    std::string itemName = extractValue(data.arguments.at("itemName"));

    get_player()->removeItemFromInventory(itemName, amount);
  }
  else if (data.opcode == Script::OP_GIVE_GOLD)
  {
    int amount = fromString<int>(extractValue(data.arguments.at("amount")));
    get_player()->gainGold(amount);
  }
  else if (data.opcode == Script::OP_TAKE_GOLD)
  {
    int amount = fromString<int>(extractValue(data.arguments.at("amount")));
    get_player()->removeGold(amount);
  }
  else if (data.opcode == Script::OP_PLAY_SOUND)
  {
    std::string sound = extractValue(data.arguments.at("sound"));
    play_sound("Audio/" + sound);
  }
  else if (data.opcode == Script::OP_ADD_PARTY_MEMBER)
  {
    int level = fromString<int>(extractValue(data.arguments.at("level")));
    std::string name = extractValue(data.arguments.at("name"));
    std::string className = extractValue(data.arguments.at("className"));

    int x = get_player()->getTrain().back()->x;
    int y = get_player()->getTrain().back()->y;

    get_player()->addNewCharacter(name, className, x, y, level);
  }
  else if (data.opcode == Script::OP_REMOVE_PARTY_MEMBER)
  {
    std::string name = extractValue(data.arguments.at("name"));

    get_player()->removeCharacter(name);
  }
  else if (data.opcode == Script::OP_SET_VISIBLE)
  {
    if (m_callingEntity)
    {
      m_callingEntity->m_visible = parseBool(extractValue(data.arguments.at("visibility")));
    }
  }
  else if (data.opcode == Script::OP_SET_WALKTHROUGH)
  {
    if (m_callingEntity)
    {
      m_callingEntity->m_walkThrough = parseBool(extractValue(data.arguments.at("walkthrough")));
    }
  }
  else if (data.opcode == Script::OP_ENABLE_CONTROLS)
  {
    get_player()->setControlsEnabled(parseBool(extractValue(data.arguments.at("enabled"))));
  }
  else if (data.opcode == Script::OP_RECOVER_ALL)
  {
    get_player()->recoverAll();
  }
  else if (data.opcode == Script::OP_COMBAT || data.opcode == Script::OP_COMBAT_NO_ESAPE)
  {
    std::vector<std::string> monsters;

    for (const auto& monsterName : data.listArguments.at("monsters"))
    {
      monsters.push_back(extractValue(monsterName));
    }

    Game::instance().startBattle(monsters, parseBool(data.arguments.at("canEscape")));
  }
  else if (data.opcode == Script::OP_ENCOUNTER)
  {
    std::string encounterName = extractValue(data.arguments.at("encounterName"));

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
    config::set(extractValue(data.arguments.at("key")), extractValue(data.arguments.at("value")));
  }
  else if (data.opcode == Script::OP_TRANSFER)
  {
    std::string targetMap = extractValue(data.arguments.at("targetMap"));
    int x = fromString<int>(extractValue(data.arguments.at("x")));
    int y = fromString<int>(extractValue(data.arguments.at("y")));
    Direction dir = directionFromString(extractValue(data.arguments.at("dir")));

    Game::instance().prepareTransfer(targetMap, x, y, dir);
  }
  else if (data.opcode == Script::OP_SHOP)
  {
    std::vector<std::string> items;

    for (const auto& item : data.listArguments.at("inventory"))
    {
      items.push_back(extractValue(item));
    }

    Game::instance().openShop(items);
  }
  else if (data.opcode == Script::OP_SHOW_PICTURE)
  {
    std::string name = extractValue(data.arguments.at("name"));
    float x = fromString<int>(extractValue(data.arguments.at("x")));
    float y = fromString<int>(extractValue(data.arguments.at("y")));

    SceneManager::instance().showPicture(name, x, y);
  }
  else if (data.opcode == Script::OP_HIDE_PICTURE)
  {
    std::string name = extractValue(data.arguments.at("name"));

    SceneManager::instance().hidePicture(name);
  }
  else if (data.opcode == Script::OP_SKILL_TRAINER)
  {
    std::vector<std::string> skills;
    for (const auto& skillName : data.listArguments.at("skills"))
    {
      skills.push_back(extractValue(skillName));
    }

    Game::instance().openSkillTrainer(skills);
  }
  else if (data.opcode == Script::OP_CAMPSITE)
  {
    Game::instance().openCampsite();
  }
  else if (data.opcode == Script::OP_SET_PLAYER_DIR)
  {
    Direction oldDir = get_player()->player()->getDirection();
    get_player()->player()->setDirection(directionFromString(extractValue(data.arguments.at("dir"))));

    // Need to update camera.
    Game::instance().fixCamera(oldDir);
  }
  else if (data.opcode == Script::OP_CHANGE_TILE)
  {
    std::string layer = extractValue(data.arguments.at("layer"));
    int x = fromString<int>(extractValue(data.arguments.at("x")));
    int y = fromString<int>(extractValue(data.arguments.at("y")));
    int tilenum = fromString<int>(extractValue(data.arguments.at("tilenum")));

    Game::instance().getCurrentMap()->setTileAt(x, y, layer, tilenum);
  }
  else if (data.opcode == Script::OP_FLASH_SCREEN)
  {
    int duration = fromString<int>(extractValue(data.arguments.at("duration")));
    uint8_t r = static_cast<uint8_t>(fromString<int>(extractValue(data.arguments.at("r"))));
    uint8_t g = static_cast<uint8_t>(fromString<int>(extractValue(data.arguments.at("g"))));
    uint8_t b = static_cast<uint8_t>(fromString<int>(extractValue(data.arguments.at("b"))));

    SceneManager::instance().flashScreen(duration, sf::Color{r, g, b});
  }
  else if (data.opcode == Script::OP_CHANGE_PLAYER_POSITION)
  {
    int x = fromString<int>(extractValue(data.arguments.at("x")));
    int y = fromString<int>(extractValue(data.arguments.at("y")));

    Game::instance().transferPlayer("", x, y);
  }
}
