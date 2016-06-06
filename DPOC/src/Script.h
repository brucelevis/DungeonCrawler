#ifndef SCRIPT_H
#define SCRIPT_H

#include <string>
#include <vector>
#include <unordered_map>

#include "Direction.h"

class Entity;
class Battle;

static const int MAX_SCRIPT_MESSAGE_BUFFER_SIZE = 512;
static const int MAX_SCRIPT_KEY_SIZE = 32;
static const int MAX_CHOICES = 4;

class Script
{
public:
  enum Opcode
  {
    OP_NOP,
    OP_MESSAGE,
    OP_WALK,
    OP_SET_DIR,
    OP_WAIT,
    OP_ASSIGNMENT,
    OP_ARITHMETIC,
    OP_IF,
    OP_END_IF,
    OP_WHILE,
    OP_WEND,
    OP_BREAK,
    OP_ELSE,
    OP_CHOICE,
    OP_SET_TILE_ID, // For entities with tileSprites.
    OP_GIVE_ITEM,
    OP_TAKE_ITEM,
    OP_GIVE_GOLD,
    OP_TAKE_GOLD,
    OP_PLAY_SOUND,
    OP_ADD_PARTY_MEMBER,
    OP_REMOVE_PARTY_MEMBER,
    OP_SET_VISIBLE,
    OP_SET_WALKTHROUGH,
    OP_ENABLE_CONTROLS,
    OP_RECOVER_ALL,
    OP_COMBAT,
    OP_COMBAT_NO_ESAPE,
    OP_ENCOUNTER,
    OP_END_GAME,
    OP_SET_CONFIG,
    OP_TRANSFER,
    OP_SHOP,
    OP_SHOW_PICTURE,
    OP_HIDE_PICTURE,
    OP_SKILL_TRAINER,
    OP_CAMPSITE,
    OP_SET_PLAYER_DIR,
    OP_CHANGE_TILE,
    OP_FLASH_SCREEN,
    OP_CHANGE_PLAYER_POSITION,
    OP_OPEN_DOOR,
    OP_CLOSE_DOOR
  };

  enum ArithmOp
  {
    ARITHM_OP_ADD,
    ARITHM_OP_SUB,
    ARITHM_OP_MUL,
    ARITHM_OP_DIV,
    ARITHM_OP_UNKNOWN
  };

  struct ScriptData
  {
    Opcode opcode;
    std::unordered_map<std::string, std::string> arguments;
    std::unordered_map<std::string, std::vector<std::string>> listArguments;
  };

  Script();

  bool loadFromFile(const std::string& file, const std::vector<std::string>& arguments = {});
  void loadFromLines(std::vector<std::string> lines, const std::vector<std::string>& arguments = {});

  bool isLoaded() const { return m_loaded; }

  void execute();
  void next();
  bool active() const;

  void setCallingEntity(Entity* entity);
  void setCallingBattle(Battle* battle);
private:
  void advance();
  void executeScriptLine();

  const ScriptData& getCurrentData() const;
  bool peekNext(ScriptData& out) const;

  ScriptData parseLine(const std::string& line, int lineNumber) const;
  Opcode getOpCode(const std::string& opStr) const;

  std::string extractValue(const std::string& input) const;
private:
  std::vector<ScriptData> m_data;
  size_t m_currentIndex;
  bool m_running;
  bool m_loaded;

  Entity* m_callingEntity;
  Battle* m_callingBattle;
};

#endif
