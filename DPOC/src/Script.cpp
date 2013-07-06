#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstring>
#include <cstdlib>
#include <cctype>

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
    if (lhs_what == "global" || lhs_what == "local" || lhs_what == "const")
    {
      lhs_key = get_value_in_bracket(lhs);
    }

    rhs_what = get_value_to_bracket(rhs);
    if (rhs_what == "global" || rhs_what == "local" || rhs_what == "const")
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

    data.data.choiceData.numberOfChoices = strings.size() - 1;
    for (size_t i = 1; i < strings.size(); i++)
    {
      strcpy(data.data.choiceData.choices[i - 1], strings[i].c_str());
    }
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
    { "message", OP_MESSAGE },
    { "walk", OP_WALK },
    { "wait", OP_WAIT },
    { "set_global", OP_SET_GLOBAL },
    { "set_local", OP_SET_LOCAL },
    { "if", OP_IF },
    { "endif", OP_END_IF },
    { "else", OP_ELSE },
    { "choice", OP_CHOICE }
  };

  auto it = OP_MAP.find(opStr);
  if (it != OP_MAP.end())
  {
    return it->second;
  }

  return OP_NOP;
}
