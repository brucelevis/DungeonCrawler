#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "coord.h"
#include "Menu.h"

class Map;
class Player;

class Game
{
public:
  static Game& instance();

  void run();

  Map* getCurrentMap() { return m_currentMap; }
  Player* getPlayer() { return m_player; }

  void transferPlayer(const std::string& targetMap, int x, int y);
  void playMusic(const std::string& music);

  void openChoiceMenu(const std::vector<std::string>& choices);

  void setPlayer(Player* player) { m_player = player; }

  void loadNewMap(const std::string& file);

  void startBattle(const std::vector<std::string>& monsters);
private:
  Game();
  ~Game();

  void pollEvents();
  void draw();
  void updatePlayer();
  bool checkWarps();

  void handleKeyPress(sf::Keyboard::Key key);

  void closeChoiceMenu();
private:
  sf::RenderWindow m_window;
  sf::RenderTexture m_targetTexture;

  Map* m_currentMap;
  Player* m_player;
  coord_t m_view;

  std::string m_currentMusicName;
  sf::Music m_currentMusic;

  MainMenu m_menu;
  ChoiceMenu* m_choiceMenu;
};

#endif
