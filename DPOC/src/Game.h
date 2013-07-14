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
struct Warp;

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

  void fadeIn(int duration);
  void fadeOut(int duration);
private:
  Game();
  ~Game();

  void update();
  void pollEvents();
  void draw();
  void updatePlayer();
  bool checkWarps();

  void handleKeyPress(sf::Keyboard::Key key);

  void closeChoiceMenu();

  void processFade();
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

  enum
  {
    FADE_NONE,
    FADE_IN,
    FADE_OUT
  } m_fade;
  int m_fadeCounter;
  int m_fadeDuration;

  const Warp* m_currentWarp;
  bool m_playerMoved;
};

#endif
