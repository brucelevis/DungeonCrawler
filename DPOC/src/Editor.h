#ifndef EDITOR_H
#define EDITOR_H

#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include "Entity.h"
#include "Config.h"
#include "Map.h"

class Editor
{
  enum TextInputState
  {
    TEXT_INPUT_NONE,
    TEXT_INPUT_RESIZE,
    TEXT_INPUT_SAVE_MAP,
    TEXT_INPUT_LOAD_MAP,
    TEXT_INPUT_SELECT_MUSIC,
    TEXT_INPUT_WARP
  };

  enum EditState
  {
    EDIT_STATE_PLACE_TILES,
    EDIT_STATE_PLACE_ENTITES,
    EDIT_STATE_PLACE_WARP,
    EDIT_STATE_PLACE_ZONE
  };
public:
  Editor();
  ~Editor();

  void run();
private:
  Editor(const Editor&);
  Editor& operator=(const Editor&);

  void clear();

  void pollEvents();

  void checkWindowEvents(sf::Event& event);
  void checkKeyEvents(sf::Event& event);
  void checkMouseEvents();

  void draw();
  void buildTileParts();

  void drawTileset();
  void drawAvailableEntities();
  void drawZones();
  void drawEditArea();

  int getNumberOfTiles() const
  {
    return m_mapW * m_mapH;
  }

  Tile* getTileAt(int x, int y, int layer);
  void updateTile(int x, int y);

  void doFloodFill(int px, int py);

  void resizeMap(int width, int height);

  void handleCarriageReturn();
  void setTextInputState(TextInputState newState);
  std::string textInputStateToString() const;
  std::string editStateToString() const;

  const Entity* getEntityAt(int x, int y) const;

  Map* createMap() const;
  void loadFromMap(Map* map);
private:
  sf::RenderWindow m_window;

  bool m_isRunning;

  sf::Texture* m_tileset;

  std::vector<Tile> m_tileParts;
  sf::IntRect m_tilesetArea;
  int m_tileScrollY;

  sf::IntRect m_editArea;
  int m_mapW, m_mapH;
  int m_scrollX, m_scrollY;
  int m_scrollXMax, m_scrollYMax;
  Tile* m_tiles[config::MAX_LAYERS];
  int m_currentLayer;

  Tile m_currentTile;

  TextInputState m_textInputState;
  std::string m_currentInput;

  std::vector<Entity*> m_availableEntities;
  std::vector<Entity*> m_entities;
  std::string m_currentEntityName;
  EditState m_editState;

  std::string m_music;

  int m_currentZone;

  std::vector<Warp> m_warps;
};

#endif
