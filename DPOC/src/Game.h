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

class Player;

class Game : public Scene
{
  static Game* theInstance;
public:
  static Game& instance();

  void update();
  void draw(sf::RenderTarget& target);
  void handleEvent(sf::Event& event);

  Map* getCurrentMap() { return m_currentMap; }
  Player* getPlayer() { return m_player; }

  void playMusic(const std::string& music);

  void openChoiceMenu(const std::vector<std::string>& choices);

  void setPlayer(Player* player) { m_player = player; }

  void loadNewMap(const std::string& file);

  void startBattle(const std::vector<std::string>& monsters, bool canEscape = true);

  void preFade(FadeType fadeType);
  void postFade(FadeType fadeType);

  void postBattle();

  void prepareTransfer(const std::string& targetMap, int x, int y);

  void openShop(const std::vector<std::string>& items);
private:
  Game();
  ~Game();

  void updatePlayer();
  bool checkWarps();

  void transferPlayer(const std::string& targetMap, int x, int y);

  void handleKeyPress(sf::Keyboard::Key key);

  void closeChoiceMenu();
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
};

#endif
