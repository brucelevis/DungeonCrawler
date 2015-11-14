#ifndef SCRIPT_H
#define SCRIPT_H

#include <string>
#include <vector>
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
    OP_SET_PLAYER_DIR
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
        char value[MAX_SCRIPT_KEY_SIZE];
        char type[MAX_SCRIPT_KEY_SIZE];
      } setPersistentData;

      struct
      {
        char key[MAX_SCRIPT_KEY_SIZE];
        char value[MAX_SCRIPT_KEY_SIZE];
        char type[MAX_SCRIPT_KEY_SIZE];
        ArithmOp operation;
      } arithmeticData;

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
        char encounterName[MAX_SCRIPT_KEY_SIZE];
      } encounterData;

      struct
      {
        char key[MAX_SCRIPT_KEY_SIZE];
        char value[MAX_SCRIPT_KEY_SIZE];
      } setConfigData;

      struct
      {
        char targetMap[MAX_SCRIPT_KEY_SIZE];
        int x, y;
        Direction dir;
      } transferData;

      struct
      {
        char inventory[32][MAX_SCRIPT_KEY_SIZE];
        int number;
      } shopData;

      struct
      {
        char name[MAX_SCRIPT_KEY_SIZE];
        float x, y;
      } showPictureData;

      struct
      {
        char name[MAX_SCRIPT_KEY_SIZE];
      } hidePictureData;

      struct
      {
        char skills[32][MAX_SCRIPT_KEY_SIZE];
        int number;
      } skillTrainer;
    } data;
  };

  Script();

  bool loadFromFile(const std::string& file);
  void loadFromLines(std::vector<std::string> lines);

  bool isLoaded() const { return m_loaded; }

  void execute();
  void next();
  bool active() const;

  void setCallingEntity(Entity* entity);
  void setCallingBattle(Battle* battle);
private:
  void advance();
  void executeScriptLine();

  ScriptData getCurrentData() const;
  bool peekNext(ScriptData& out) const;

  ScriptData parseLine(const std::string& line) const;
  Opcode getOpCode(const std::string& opStr) const;
private:
  std::vector<ScriptData> m_data;
  size_t m_currentIndex;
  bool m_running;
  bool m_loaded;

  Entity* m_callingEntity;
  Battle* m_callingBattle;
};

#endif
