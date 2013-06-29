#ifndef BATTLE_H
#define BATTLE_H

#include <vector>
#include <map>

#include <SFML/Graphics.hpp>

#include "Menu.h"

class Character;

class Battle
{
  enum State
  {
    STATE_SELECT_ACTIONS,
    STATE_EXECUTE_ACTIONS,
    STATE_SHOW_ACTION,
    STATE_ACTION_EFFECT
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

  void start();

  void setAction(Character* user, Action action);

  void doneSelectingActions();
private:
  void executeActions();

  void pollEvents();

  void handleKeyPress(sf::Keyboard::Key key);

  void draw();

  void addToBattleOrder(Character* character);
private:
  bool m_battleOngoing;
  State m_state;

  BattleMenu m_battleMenu;
  std::vector<Character*> m_monsters;
  std::vector<Character*> m_battleOrder;
  std::map<Character*, Action> m_battleActions;
  Character* m_currentActor;

  sf::RenderWindow& m_window;
};

#endif
