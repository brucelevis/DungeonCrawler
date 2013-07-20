#ifndef SCRIPT_H
#define SCRIPT_H

#include <string>
#include <vector>
#include "Direction.h"

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
    OP_SET_GLOBAL,
    OP_SET_LOCAL,
    OP_IF,
    OP_END_IF,
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
    OP_END_GAME,
    OP_SET_CONFIG,
    OP_TRANSFER
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

      struct
      {
        int numberOfChoices;
        char choices[MAX_CHOICES][MAX_SCRIPT_KEY_SIZE];
      } choiceData;

      struct
      {
        int tileId;
      } setTileIdData;

      struct
      {
        char itemName[MAX_SCRIPT_KEY_SIZE];
        int amount;
      } giveItemData;

      struct
      {
        int amount;
      } giveGoldData;

      struct
      {
        char sound[MAX_SCRIPT_KEY_SIZE];
      } playSoundData;

      struct
      {
        char name[MAX_SCRIPT_KEY_SIZE];
        char className[MAX_SCRIPT_KEY_SIZE];
        int level;
      } addPartyMemberData;

      struct
      {
        char name[MAX_SCRIPT_KEY_SIZE];
      } removePartyMemberData;

      struct
      {
        bool visibility;
      } setVisibleData;

      struct
      {
        bool walkthrough;
      } setWalkthroughData;

      struct
      {
        bool enabled;
      } enableControlsData;

      struct
      {
        int number;
        char monsters[MAX_SCRIPT_KEY_SIZE][MAX_SCRIPT_KEY_SIZE];
        bool canEscape;
      } combatData;

      struct
      {
        char key[MAX_SCRIPT_KEY_SIZE];
        char value[MAX_SCRIPT_KEY_SIZE];
      } setConfigData;

      struct
      {
        char targetMap[MAX_SCRIPT_KEY_SIZE];
        int x, y;
      } transferData;
    } data;
  };

  Script();

  bool loadFromFile(const std::string& file);
  void loadFromLines(std::vector<std::string> lines);

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
