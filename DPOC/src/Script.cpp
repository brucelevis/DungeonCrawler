#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstring>

#include "logger.h"
#include "Utility.h"
#include "Script.h"

Script::Script()
 : m_currentIndex(0),
   m_running(false),
   m_loaded(false)
{

}

void Script::loadFromFile(const std::string& file)
{
  m_currentIndex = 0;
  m_loaded = true;

  TRACE("Loading script %s", file.c_str());

  std::ifstream infile(file.c_str());
  if (infile.is_open())
  {
    std::vector<std::string> lines = get_lines(infile);

    for (auto it = lines.begin(); it != lines.end(); ++it)
    {
      if (it->size() > 0)
      {
        ScriptData data = parseLine(*it);
        m_data.push_back(data);
      }
    }

    infile.close();
  }
  else
  {
    TRACE("Unable to open %s", file.c_str());
  }
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

  // Comment
  if (strings[0][0] == '#')
    return ScriptData();

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
    { "message", OP_MESSAGE }
  };

  auto it = OP_MAP.find(opStr);
  if (it != OP_MAP.end())
  {
    return it->second;
  }

  return OP_NOP;
}
