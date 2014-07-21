#ifndef BATTLE_H
#define BATTLE_H

#include <vector>
#include <map>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "Menu.h"
#include "Target.h"
#include "Effect.h"
#include "Scene.h"

class Character;
class PlayerCharacter;

class Battle : public Scene
{
  enum State
  {
    STATE_BATTLE_BEGINS,
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

  Battle(const std::vector<Character*>& monsters);

  ~Battle();

  void start(bool canEscape = true);

  void update();
  void draw(sf::RenderTarget& target);
  void handleEvent(sf::Event& event);

  void setAction(Character* user, Action action);

  void doneSelectingActions();

  void endBattle();

  void postFade(FadeType fadeType);

  void setBattleBackground(const std::string& file);
private:
  void executeActions();
  void showAction();
  void actionEffect();
  void doVictory();
  void processStatusEffects();

  bool processStatusEffectForCharacter(Character* actor);

  void pollEvents();

  void handleKeyPress(sf::Keyboard::Key key);

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
private:
  bool m_battleOngoing;
  State m_state;

  BattleMenu m_battleMenu;
  std::vector<Character*> m_monsters;
  std::vector<Character*> m_battleOrder;
  std::map<Character*, std::vector<Action> > m_battleActions;
  Character* m_currentActor;

  std::vector<Character*> m_currentTargets;
  std::vector<BattleAnimation*> m_activeEffects;

  // A short delay between "damage" and "next actor".
  int m_turnDelay;

  sf::Music m_battleMusic;

  bool m_canEscape;

  sf::Texture* m_battleBackground;

  float m_battleBeginFade;
};

#endif
