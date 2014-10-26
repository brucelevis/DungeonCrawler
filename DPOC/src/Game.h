#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "coord.h"
#include "Menu.h"
#include "Scene.h"
#include "Map.h"
#include "Camera.h"

#include "Minimap.h"

class Player;
class Entity;
class Raycaster;

class Game : public Scene
{
  static Game* theInstance;
public:
  static Game& instance();

  void start(Player* thePlayer);

  void update();
  void draw(sf::RenderTarget& target);
  void handleEvent(sf::Event& event);

  Map* getCurrentMap() { return m_currentMap; }
  Player* getPlayer() { return m_player; }

  void playMusic(const std::string& music);

  void openChoiceMenu(const std::vector<std::string>& choices);

  void setPlayer(Player* player);

  void loadNewMap(const std::string& file);

  void startBattle(const std::vector<std::string>& monsters, bool canEscape = true, const std::string& music = "");

  void preFade(FadeType fadeType);
  void postFade(FadeType fadeType);

  void postBattle();

  void prepareTransfer(const std::string& targetMap, int x, int y);

  void openShop(const std::vector<std::string>& items);
  void openSkillTrainer(const std::vector<std::string>& skills);
  void openCampsite();

  bool battleInProgress() const { return m_battleInProgress; }
private:
  Game();
  ~Game();

  void updatePlayer();
  bool checkWarps();
  bool checkTraps();
  bool checkInteractions();

  void transferPlayer(const std::string& targetMap, int x, int y);

  void handleKeyPress(sf::Keyboard::Key key);

  void closeChoiceMenu();

  void startRotate(int angle, int angleInc);
  void execRotate();

  void drawParty(sf::RenderTarget& target) const;
private:
  Map* m_currentMap;
  Player* m_player;
  coord_t m_view;

  std::string m_currentMusicName;
  sf::Music m_currentMusic;

  MainMenu m_menu;
  ChoiceMenu* m_choiceMenu;

  bool m_transferInProgress;
  Warp m_currentWarp;
  bool m_playerMoved;

  std::vector<Entity*> m_entitiesToDraw;

  Camera m_camera;
  Raycaster* m_raycaster;

  bool m_isRotating;
  int m_angleToRotate;
  int m_angleInc;
  int m_accumulatedAngle;
  float m_rotateDegs;

  sf::Image m_raycasterBuffer;
  sf::Texture m_texture;
  sf::RenderTexture m_targetTexture;

  bool m_rotKeyDown;

  Minimap m_minimap;
  bool m_battleInProgress;

  std::string m_savedBattleMusic;

  bool m_campSite;
};

#endif
