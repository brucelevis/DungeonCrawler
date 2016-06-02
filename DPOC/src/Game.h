#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "coord.h"
#include "Menu.h"
#include "Scene.h"
#include "Map.h"
#include "Camera.h"
#include "MapRenderer.h"

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
  void draw(sf::RenderTexture& target);
  void handleEvent(sf::Event& event);

  Map* getCurrentMap() { return m_currentMap; }
  Player* getPlayer() { return m_player; }

  void playMusic(const std::string& music);

  void openChoiceMenu(const std::vector<std::string>& choices);

  void setPlayer(Player* player);

  void loadNewMap(const std::string& file);

  void startBattle(const std::vector<std::string>& monsters, bool canEscape = true, const std::string& music = "", const std::vector<std::string>& script = {});

  void preFade(FadeType fadeType);
  void postFade(FadeType fadeType);

  void postBattle();

  void prepareTransfer(const std::string& targetMap, int x, int y, Direction dir = DIR_RANDOM);

  void openShop(const std::vector<std::string>& items);
  void openSkillTrainer(const std::vector<std::string>& skills);
  void openCampsite();
  void openMap();

  bool battleInProgress() const { return m_battleInProgress; }

  void transferPlayer(const std::string& targetMap, int x, int y);
private:
  Game();
  ~Game();

  void updatePlayer();
  bool checkWarps();
  bool checkTraps();
  bool checkInteractions();

  void handleKeyPress(sf::Keyboard::Key key);

  void closeChoiceMenu();

  void drawParty(sf::RenderTarget& target) const;
private:
  glm::mat4 m_projectionMatrix;

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

  std::unique_ptr<MapRenderer> m_mapRenderer;

  sf::Texture m_texture;
  sf::RenderTexture m_targetTexture;

  bool m_rotKeyDown;

  Minimap m_minimap;
  bool m_battleInProgress;

  std::string m_savedBattleMusic;

  bool m_campSite;
};

#endif
