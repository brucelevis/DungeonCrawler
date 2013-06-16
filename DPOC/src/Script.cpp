#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstring>
#include <cstdlib>

#include "logger.h"
#include "Utility.h"
#include "Script.h"

static void strip_comments(std::vector<std::string>& lines)
{
  for (auto it = lines.begin(); it != lines.end();)
  {
    if (it->size() == 0)
    {
      it = lines.erase(it);
    }
    else if (it->at(0) == '#')
    {
      it = lines.erase(it);
    }
    else
    {
      ++it;
    }
  }
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

    strip_comments(lines);

    for (auto it = lines.begin(); it != lines.end(); ++it)
    {
      ScriptData data = parseLine(*it);
      m_data.push_back(data);
    }

    infile.close();

    return true;
  }
  else
  {
    TRACE("Unable to open %s", file.c_str());
  }

  return false;
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
  else if (opcode == OP_SET_GLOBAL_INT)
  {
    std::string key = strings[1];
    int value = atoi(strings[2].c_str());

    memset(data.data.setGlobalIntData.key, 0, MAX_SCRIPT_KEY_SIZE);

    strcpy(data.data.setGlobalIntData.key, key.c_str());
    data.data.setGlobalIntData.value = value;
  }
  else if (opcode == OP_SET_LOCAL_INT)
  {
    int value = atoi(strings[1].c_str());

    data.data.setLocalIntData.value = value;
  }
  else if (opcode == OP_TOGGLE_GLOBAL)
  {
    std::string key = strings[1];

    memset(data.data.toggleGlobalData.key, 0, MAX_SCRIPT_KEY_SIZE);
    strcpy(data.data.toggleGlobalData.key, key.c_str());
  }
  else if (opcode == OP_TOGGLE_LOCAL)
  {
    // Nothing
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
    { "set_global_int", OP_SET_GLOBAL_INT },
    { "set_local_int", OP_SET_LOCAL_INT },
    { "toggle_global", OP_TOGGLE_GLOBAL },
    { "toggle_local", OP_TOGGLE_LOCAL }
  };

  auto it = OP_MAP.find(opStr);
  if (it != OP_MAP.end())
  {
    return it->second;
  }

  return OP_NOP;
}
