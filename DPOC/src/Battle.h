#ifndef BATTLE_H
#define BATTLE_H

#include <vector>
#include <map>

#include <SFML/Graphics.hpp>

#include "Menu.h"
#include "Target.h"

class Character;

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
    STATE_DEFEAT
  };
public:
  struct Action
  {
    std::string actionName;
    Character* target;  // TODO: Vector

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

  void pollEvents();

  void handleKeyPress(sf::Keyboard::Key key);

  void draw();

  void addToBattleOrder(Character* character);

  void updateEffects();
  bool effectInProgress() const;

  bool isMonster(Character* actor);

  void nextActor();

  Character* selectRandomTarget(Character* actor);

  bool allDead(const std::vector<Character*>& actors) const;

  std::vector<Character*> getAliveActors(const std::vector<Character*>& actors);
  void setCurrentTargets(Target targetType);

  int getExperience() const;
  int getGold() const;
private:
  bool m_battleOngoing;
  State m_state;

  BattleMenu m_battleMenu;
  std::vector<Character*> m_monsters;
  std::vector<Character*> m_battleOrder;
  std::map<Character*, Action> m_battleActions;
  Character* m_currentActor;

  std::vector<Character*> m_currentTargets;

  sf::RenderWindow& m_window;

  // A short delay between "damage" and "next actor".
  int m_turnDelay;
};

#endif
