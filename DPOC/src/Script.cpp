#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <stdexcept>

#include "logger.h"
#include "Utility.h"
#include "Script.h"

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

Script::Script()
 : m_currentIndex(0),
   m_running(false),
   m_loaded(false)
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

  Opcode opcode = getOpCode(strings[0]);

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
  else if (opcode == OP_SET_GLOBAL)
  {
    std::string key = strings[1];
    int value;

    if (strings[2] == "true" || strings[2] == "false")
    {
      value = strings[2] == "true";
    }
    else
    {
      value = atoi(strings[2].c_str());
    }

    memset(data.data.setGlobalData.key, 0, MAX_SCRIPT_KEY_SIZE);

    strcpy(data.data.setGlobalData.key, key.c_str());
    data.data.setGlobalData.value = value;
  }
  else if (opcode == OP_SET_LOCAL)
  {
    std::string key = strings[1];
    int value;

    if (strings[2] == "true" || strings[2] == "false")
    {
      value = strings[2] == "true";
    }
    else
    {
      value = atoi(strings[2].c_str());
    }

    memset(data.data.setLocalData.key, 0, MAX_SCRIPT_KEY_SIZE);

    strcpy(data.data.setLocalData.key, key.c_str());
    data.data.setLocalData.value = value;
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

    lhs_what = get_value_to_bracket(lhs);
    if (lhs_what == "global" || lhs_what == "local" || lhs_what == "const" || lhs_what == "item")
    {
      lhs_key = get_value_in_bracket(lhs);
    }

    rhs_what = get_value_to_bracket(rhs);
    if (rhs_what == "global" || rhs_what == "local" || rhs_what == "const" || rhs_what == "item")
    {
      rhs_key = get_value_in_bracket(rhs);
    }

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
  else if (opcode == OP_COMBAT)
  {
    memset(data.data.combatData.monsters, '\0', MAX_SCRIPT_KEY_SIZE * MAX_SCRIPT_KEY_SIZE);

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
      strcpy(data.data.combatData.monsters[i], monsters[i].c_str());
    }

    data.data.combatData.number = monsters.size();
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
    { "set_global",   OP_SET_GLOBAL },
    { "set_local",    OP_SET_LOCAL },
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
    { "set_visible",  OP_SET_VISIBLE },
    { "set_walkthrough", OP_SET_WALKTHROUGH },
    { "enable_controls", OP_ENABLE_CONTROLS },
    { "recover_all",  OP_RECOVER_ALL },
    { "combat",       OP_COMBAT }
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
