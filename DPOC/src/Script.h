#ifndef SCRIPT_H
#define SCRIPT_H

#include <string>
#include <vector>
#include "Direction.h"

static const int MAX_SCRIPT_MESSAGE_BUFFER_SIZE = 512;
static const int MAX_SCRIPT_KEY_SIZE = 32;

class Script
{
public:
  enum Opcode
  {
    OP_NOP,
    OP_MESSAGE,
    OP_WALK,
    OP_WAIT,
    OP_SET_GLOBAL,
    OP_SET_LOCAL,
    OP_IF,
    OP_END_IF,
    OP_ELSE
  };

  struct ScriptData
  {
    Opcode opcode;

    union
    {
      struct
      {
        char message[MAX_SCRIPT_MESSAGE_BUFFER_SIZE];
      } messageData;

      struct
      {
        Direction dir;
      } walkData;

      struct
      {
        int duration;
      } waitData;

      struct
      {
        char key[MAX_SCRIPT_KEY_SIZE];
        int value;
      } setGlobalData;

      struct
      {
        char key[MAX_SCRIPT_KEY_SIZE];
        int value;
      } setLocalData;

      struct
      {
        char lhs[MAX_SCRIPT_KEY_SIZE];
        char rhs[MAX_SCRIPT_KEY_SIZE];

        char lhsKey[MAX_SCRIPT_KEY_SIZE];
        char rhsKey[MAX_SCRIPT_KEY_SIZE];

        char boolOperation[MAX_SCRIPT_KEY_SIZE];
      } ifData;
    } data;
  };

  Script();

  bool loadFromFile(const std::string& file);

  bool isLoaded() const { return m_loaded; }

  void execute();
  void advance();
  void stepBack();
  bool active() const;
  ScriptData getCurrentData() const;

  bool peekNext(ScriptData& out) const;
private:
  ScriptData parseLine(const std::string& line) const;
  Opcode getOpCode(const std::string& opStr) const;
private:
  std::vector<ScriptData> m_data;
  size_t m_currentIndex;
  bool m_running;
  bool m_loaded;
};

#endif
