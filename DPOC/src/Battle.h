#ifndef BATTLE_H
#define BATTLE_H

#include <vector>
#include <map>

#include <SFML/Graphics.hpp>

#include "Menu.h"
#include "Target.h"
#include "Effect.h"

class Character;
class PlayerCharacter;

class Battle
{
  enum State
  {
    STATE_SELECT_ACTIONS,
    STATE_EXECUTE_ACTIONS,
    STATE_SHOW_ACTION,
    STATE_ACTION_EFFECT,
    STATE_EFFECT_MESSAGE,
    STATE_VICTORY_PRE,
    STATE_VICTORY_POST,
    STATE_DEFEAT,
    STATE_ESCAPE,
    STATE_PROCESS_STATUS_EFFECTS
  };
public:
  struct Action
  {
    std::string actionName;
    Character* target;

    // Item or spell.
    std::string objectName;
  };

  Battle(sf::RenderWindow& window, const std::vector<Character*>& monsters);

  ~Battle();

  void start();

  void setAction(Character* user, Action action);

  void doneSelectingActions();

  void endBattle();
private:
  void executeActions();
  void showAction();
  void actionEffect();
  void doVictory();
  void processStatusEffects();

  bool processStatusEffectForCharacter(Character* actor);

  void pollEvents();

  void handleKeyPress(sf::Keyboard::Key key);

  void draw();

  void addToBattleOrder(Character* character);

  void updateEffects();
  bool effectInProgress() const;

  bool isMonster(Character* actor);

  void nextActor();

  Character* selectRandomTarget(Character* actor);
  Character* selectRandomFriendlyTarget(Character* actor);

  void setCurrentTargets(Target targetType);
  void createEffects();

  int getExperience() const;
  int getGold() const;

  std::vector<Character*> getAllActors() const;

  bool checkVictoryOrDefeat();

  void shakeScreen();
private:
  bool m_battleOngoing;
  State m_state;

  BattleMenu m_battleMenu;
  std::vector<Character*> m_monsters;
  std::vector<Character*> m_battleOrder;
  std::map<Character*, Action> m_battleActions;
  Character* m_currentActor;

  std::vector<Character*> m_currentTargets;
  std::vector<Effect*> m_activeEffects;

  sf::RenderTexture m_targetTexture;
  sf::RenderWindow& m_window;

  // A short delay between "damage" and "next actor".
  int m_turnDelay;

  int m_shakeCounter;
};

#endif
