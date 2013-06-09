#ifndef EDITOR_H
#define EDITOR_H

#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include "Entity.h"
#include "Config.h"

class Editor
{
  struct TilePart
  {
    int tileX, tileY;
  };

  enum TextInputState
  {
    TEXT_INPUT_NONE,
    TEXT_INPUT_RESIZE,
    TEXT_INPUT_SAVE_MAP,
    TEXT_INPUT_LOAD_MAP,
    TEXT_INPUT_SELECT_MUSIC
  };

  enum EditState
  {
    EDIT_STATE_PLACE_TILES,
    EDIT_STATE_PLACE_ENTITES
  };
public:
  Editor();
  ~Editor();

  void run();
private:
  Editor(const Editor&);
  Editor& operator=(const Editor&);

  void pollEvents();

  void checkWindowEvents(sf::Event& event);
  void checkKeyEvents(sf::Event& event);
  void checkMouseEvents();

  void draw();
  void buildTileParts();

  void drawTileset();
  void drawAvailableEntities();
  void drawEditArea();

  int getNumberOfTiles() const
  {
    return m_mapW * m_mapH;
  }

  const TilePart* getTileAt(int x, int y, int layer) const;
  void updateTile(int x, int y);

  void doFloodFill(int px, int py);

  void resizeMap(int width, int height);

  void handleCarriageReturn();
  void setTextInputState(TextInputState newState);
  std::string textInputStateToString() const;

  const Entity* getEntityAt(int x, int y) const;
private:
  sf::RenderWindow m_window;

  bool m_isRunning;

  sf::Texture* m_tileset;

  std::vector<TilePart> m_tileParts;
  sf::IntRect m_tilesetArea;
  int m_tileScrollY;

  sf::IntRect m_editArea;
  int m_mapW, m_mapH;
  int m_scrollX, m_scrollY;
  int m_scrollXMax, m_scrollYMax;
  TilePart* m_tiles[config::MAX_LAYERS];
  int m_currentLayer;

  TilePart m_currentTile;

  TextInputState m_textInputState;
  std::string m_currentInput;

  std::vector<Entity*> m_entities;
  std::string m_currentEntityName;
  EditState m_editState;
};

#endif
