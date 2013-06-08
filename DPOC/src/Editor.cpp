#include <algorithm>
#include <sstream>

#include <SFML/Graphics.hpp>

#include "logger.h"
#include "floodfill.h"
#include "draw_text.h"
#include "Cache.h"
#include "EntityDef.h"
#include "Editor.h"

Editor::Editor()
 : m_isRunning(true),
   m_tilesetArea(0, 0, 64, 480),
   m_tileScrollY(0),
   m_editArea(80, 32, 512, 416),
   m_mapW(20), m_mapH(20),
   m_scrollX(0), m_scrollY(0),
   m_scrollXMax(0), m_scrollYMax(0),
   m_currentLayer(0),
   m_currentTile({0, 0}),
   m_textInputState(TEXT_INPUT_NONE),
   m_editState(EDIT_STATE_PLACE_TILES)
{
  m_window.create(sf::VideoMode(640, 480), "DPOC Editor");

  m_tileset = cache::loadTexture("Resources/DqTileset.png");

  if (!m_tileset)
  {
    TRACE("Unable to load tileset Resources/DqTileset.png");
  }
  else
  {
    buildTileParts();
  }

  for (int i = 0; i < MAX_LAYERS; i++)
  {
    m_tiles[i] = new TilePart[m_mapW * m_mapH];

    for (int j = 0; j < getNumberOfTiles(); j++)
    {
      m_tiles[i][j].tileX = m_currentTile.tileX;
      m_tiles[i][j].tileY = m_currentTile.tileY;
    }
  }
}

Editor::~Editor()
{
  m_window.close();
  cache::releaseTexture("Resources/DqTileset.png");
  for (int i = 0; i < MAX_LAYERS; i++)
    delete[] m_tiles[i];
  for (auto it = m_entities.begin(); it != m_entities.end(); ++it)
    delete *it;
}

void Editor::run()
{
  while (m_isRunning)
  {
    pollEvents();

    checkMouseEvents();

    m_scrollX = std::min(m_mapW - m_editArea.width / TILE_W, m_scrollX);
    m_scrollY = std::min(m_mapH - m_editArea.height / TILE_H, m_scrollY);
    m_scrollX = std::max(0, m_scrollX);
    m_scrollY = std::max(0, m_scrollY);

    m_scrollXMax = m_scrollX + m_editArea.width / TILE_W;
    m_scrollYMax = m_scrollY + m_editArea.height / TILE_H;
    if (m_scrollXMax > m_mapW) m_scrollXMax = m_mapW;
    if (m_scrollYMax > m_mapH) m_scrollYMax = m_mapH;

    m_tileScrollY = std::max(0, m_tileScrollY);

    draw();
  }
}

void Editor::pollEvents()
{
  sf::Event event;

  TextInputState prevState = m_textInputState;

  while (m_window.pollEvent(event))
  {
    checkWindowEvents(event);
    checkKeyEvents(event);
  }

  if (prevState != m_textInputState)
  {
    m_currentInput.clear();
  }
}

void Editor::checkWindowEvents(sf::Event& event)
{
  switch (event.type)
  {
  case sf::Event::Closed:
    m_isRunning = false;
    break;
  default:
    break;
  }
}

void Editor::checkKeyEvents(sf::Event& event)
{
  if (m_textInputState == TEXT_INPUT_NONE && event.type == sf::Event::KeyPressed)
  {
    if (event.key.code == sf::Keyboard::Right)
    {
      m_scrollX++;
    }
    else if (event.key.code == sf::Keyboard::Left)
    {
      m_scrollX--;
    }
    else if (event.key.code == sf::Keyboard::Down)
    {
      m_scrollY++;
    }
    else if (event.key.code == sf::Keyboard::Up)
    {
      m_scrollY--;
    }

    if (event.key.code == sf::Keyboard::A)
    {
      m_tileScrollY++;
    }
    else if (event.key.code == sf::Keyboard::Q)
    {
      m_tileScrollY--;
    }

    if (event.key.code == sf::Keyboard::F5)
    {
      m_currentLayer = 0;

      // Reset scrolling if changing back to tile mode.
      if (m_editState == EDIT_STATE_PLACE_ENTITES)
      {
        m_tileScrollY = 0;
        m_editState = EDIT_STATE_PLACE_TILES;
      }
    }
    else if (event.key.code == sf::Keyboard::F6)
    {
      m_currentLayer = 1;

      // Reset scrolling if changing back to tile mode.
      if (m_editState == EDIT_STATE_PLACE_ENTITES)
      {
        m_tileScrollY = 0;
        m_editState = EDIT_STATE_PLACE_TILES;
      }
    }
    else if (event.key.code == sf::Keyboard::F7)
    {
      m_tileScrollY = 0;
      m_editState = EDIT_STATE_PLACE_ENTITES;
    }

    if (event.key.code == sf::Keyboard::F && m_editState == EDIT_STATE_PLACE_TILES)
    {
      int mouseX = sf::Mouse::getPosition(m_window).x;
      int mouseY = sf::Mouse::getPosition(m_window).y;

      if (m_editArea.contains(mouseX, mouseY))
      {
        int px = m_scrollX + (mouseX - m_editArea.left) / 16;
        int py = m_scrollY + (mouseY - m_editArea.top) / 16;

        doFloodFill(px, py);
      }
    }

    if (event.key.code == sf::Keyboard::R)
    {
      setTextInputState(TEXT_INPUT_RESIZE);
    }
  }
  else if (m_textInputState != TEXT_INPUT_NONE && event.type == sf::Event::TextEntered)
  {
    if (event.text.unicode < 128)
    {
      char c = (char)event.text.unicode;

      if (c == '\b')
      {
        if (m_currentInput.size() > 0)
        {
          m_currentInput.resize(m_currentInput.size() - 1);
        }
      }
      else if (c == '\n' || c == '\r')
      {
        handleCarriageReturn();

        setTextInputState(TEXT_INPUT_NONE);
      }
      else
      {
        m_currentInput += c;
      }

    }
  }
}

void Editor::checkMouseEvents()
{
  if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
  {
    int mouseX = sf::Mouse::getPosition(m_window).x;
    int mouseY = sf::Mouse::getPosition(m_window).y;

    if (m_tilesetArea.contains(mouseX, mouseY))
    {
      int px = (mouseX - m_tilesetArea.left) / 16;
      int py = m_tileScrollY + (mouseY - m_tilesetArea.top) / 16;

      if (m_editState == EDIT_STATE_PLACE_TILES)
      {
        int index = py * (m_tilesetArea.width / TILE_W) + px;
        if (index >= 0 && index < (int)m_tileParts.size())
        {
          m_currentTile = m_tileParts[index];
        }
      }
      else if (m_editState == EDIT_STATE_PLACE_ENTITES)
      {
        int index = py * (m_tilesetArea.width / TILE_W) + px;
        if (index >= 0 && index < (int)ENTITY_DEF.size())
        {
          m_currentEntityName = ENTITY_DEF.at(index).name;
        }
      }
    }
    else if (m_editArea.contains(mouseX, mouseY))
    {
      int px = m_scrollX + (mouseX - m_editArea.left) / 16;
      int py = m_scrollY + (mouseY - m_editArea.top) / 16;

      if (m_editState == EDIT_STATE_PLACE_TILES)
      {
        if (getTileAt(px, py, m_currentLayer))
        {
          updateTile(px, py);
        }
      }
      else if (m_editState == EDIT_STATE_PLACE_ENTITES)
      {
        if (!getEntityAt(px, py))
        {
          TRACE("Placing entity: %s at [%d, %d]", m_currentEntityName.c_str(), px, py);

          Entity* entity = create_entity(m_currentEntityName);
          entity->x = px;
          entity->y = py;
          m_entities.push_back(entity);
        }
      }
    }
  }
  else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
  {
    int mouseX = sf::Mouse::getPosition(m_window).x;
    int mouseY = sf::Mouse::getPosition(m_window).y;

    if (m_editArea.contains(mouseX, mouseY))
    {
      int px = m_scrollX + (mouseX - m_editArea.left) / 16;
      int py = m_scrollY + (mouseY - m_editArea.top) / 16;

      if (m_editState == EDIT_STATE_PLACE_TILES)
      {
        const TilePart* tile = getTileAt(px, py, m_currentLayer);
        if (tile)
        {
          m_currentTile.tileX = tile->tileX;
          m_currentTile.tileY = tile->tileY;
        }
      }
      else if (m_editState == EDIT_STATE_PLACE_ENTITES)
      {
        const Entity* entity = getEntityAt(px, py);
        if (entity)
        {
          TRACE("Removing entity: %s at [%d, %d]", entity->name.c_str(), px, py);

          m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), entity), m_entities.end());
          delete entity;
        }
      }
    }
  }
}

void Editor::draw()
{
  m_window.clear();

  if (m_editState == EDIT_STATE_PLACE_TILES)
  {
    drawTileset();
  }
  else if (m_editState == EDIT_STATE_PLACE_ENTITES)
  {
    drawAvailableEntities();
  }

  drawEditArea();

  if (m_textInputState == TEXT_INPUT_RESIZE)
  {
    draw_text(m_window, m_tilesetArea.left + m_tilesetArea.width + 8, m_tilesetArea.top + 4,
        "Current size (%d %d) New size: %s_", m_mapW, m_mapH, m_currentInput.c_str());
  }

  m_window.display();
}

void Editor::drawTileset()
{
  sf::Sprite sprite;
  sprite.setTexture(*m_tileset);

  int x = 0;
  int y = 0;

  for (size_t i = 0; i < m_tileParts.size(); i++)
  {
    const TilePart& tp = m_tileParts[i];
    int posX = x * TILE_W;
    int posY = (y - m_tileScrollY) * TILE_H;

    if (posY >= (m_tilesetArea.top + m_tilesetArea.height))
      continue;

    posX += m_tilesetArea.left;
    posY += m_tilesetArea.top;

    sprite.setTextureRect(sf::IntRect(tp.tileX*TILE_W, tp.tileY*TILE_H, TILE_W, TILE_H));
    sprite.setPosition(posX, posY);
    m_window.draw(sprite);

    if (tp.tileX == m_currentTile.tileX && tp.tileY == m_currentTile.tileY)
    {
      sf::RectangleShape rect;
      rect.setPosition(posX + 2, posY + 2);
      rect.setSize(sf::Vector2f(TILE_W - 4, TILE_H - 4));
      rect.setFillColor(sf::Color::Transparent);
      rect.setOutlineColor(sf::Color::Green);
      rect.setOutlineThickness(2.0f);
      m_window.draw(rect);
    }

    int mouseX = sf::Mouse::getPosition(m_window).x;
    int mouseY = sf::Mouse::getPosition(m_window).y;
    if (mouseX > posX && mouseY > posY && mouseX < (posX + TILE_W) && mouseY < (posY + TILE_H))
    {
      sf::RectangleShape rect;
      rect.setPosition(posX + 2, posY + 2);
      rect.setSize(sf::Vector2f(TILE_W - 4, TILE_H - 4));
      rect.setFillColor(sf::Color::Transparent);
      rect.setOutlineColor(sf::Color::Red);
      rect.setOutlineThickness(2.0f);
      m_window.draw(rect);
    }

    x++;
    if ((x % (m_tilesetArea.width / TILE_W)) == 0)
    {
      x = 0;
      y++;
    }
  }
}

void Editor::drawAvailableEntities()
{
//  sf::Sprite sprite;
//  sprite.setTexture(*m_tileset);

  int x = 0;
  int y = 0;

  for (auto it = ENTITY_DEF.begin(); it != ENTITY_DEF.end(); ++it)
  {
    int posX = x * TILE_W;
    int posY = (y - m_tileScrollY) * TILE_H;

    if (posY >= (m_tilesetArea.top + m_tilesetArea.height))
      continue;

    posX += m_tilesetArea.left;
    posY += m_tilesetArea.top;

//    sprite.setTextureRect(sf::IntRect(tp.tileX*TILE_W, tp.tileY*TILE_H, TILE_W, TILE_H));
//    sprite.setPosition(posX, posY);
//    m_window.draw(sprite);
    sf::RectangleShape tmpRect;
    tmpRect.setPosition(posX, posY);
    tmpRect.setSize(sf::Vector2f(TILE_W, TILE_H));
    tmpRect.setFillColor(sf::Color::Blue);
    tmpRect.setOutlineColor(sf::Color::Transparent);
    m_window.draw(tmpRect);

    if (m_currentEntityName == it->name)
    {
      sf::RectangleShape rect;
      rect.setPosition(posX + 2, posY + 2);
      rect.setSize(sf::Vector2f(TILE_W - 4, TILE_H - 4));
      rect.setFillColor(sf::Color::Transparent);
      rect.setOutlineColor(sf::Color::Green);
      rect.setOutlineThickness(2.0f);
      m_window.draw(rect);
    }

    int mouseX = sf::Mouse::getPosition(m_window).x;
    int mouseY = sf::Mouse::getPosition(m_window).y;
    if (mouseX > posX && mouseY > posY && mouseX < (posX + TILE_W) && mouseY < (posY + TILE_H))
    {
      sf::RectangleShape rect;
      rect.setPosition(posX + 2, posY + 2);
      rect.setSize(sf::Vector2f(TILE_W - 4, TILE_H - 4));
      rect.setFillColor(sf::Color::Transparent);
      rect.setOutlineColor(sf::Color::Red);
      rect.setOutlineThickness(2.0f);
      m_window.draw(rect);

      draw_text(m_window, m_tilesetArea.left + m_tilesetArea.width + 8, m_tilesetArea.top + 4, "Entity: %s", it->name.c_str());
    }

    x++;
    if ((x % (m_tilesetArea.width / TILE_W)) == 0)
    {
      x = 0;
      y++;
    }
  }
}

void Editor::drawEditArea()
{
  sf::RectangleShape rect;
  rect.setPosition(m_editArea.left, m_editArea.top);
  rect.setSize(sf::Vector2f(m_editArea.width, m_editArea.height));
  rect.setFillColor(sf::Color::Transparent);
  rect.setOutlineColor(sf::Color::White);
  rect.setOutlineThickness(1.0f);
  m_window.draw(rect);

  sf::Sprite sprite;
  sprite.setTexture(*m_tileset);

  for (int y = m_scrollY; y < m_scrollYMax; y++)
  {
    for (int x = m_scrollX; x < m_scrollXMax; x++)
    {
      int posX = TILE_W*(x - m_scrollX) + m_editArea.left;
      int posY = TILE_H*(y - m_scrollY) + m_editArea.top;

      for (int i = 0; i < MAX_LAYERS; i++)
      {
        const TilePart* tp = getTileAt(x, y, i);
        if (tp)
        {
          sprite.setPosition(posX, posY);
          sprite.setTextureRect(sf::IntRect(tp->tileX*TILE_W, tp->tileY*TILE_H, TILE_W, TILE_H));

          if (i != m_currentLayer)
          {
            sprite.setColor(sf::Color(255, 255, 255, 127));
          }
          else
          {
            sprite.setColor(sf::Color::White);
          }

          m_window.draw(sprite);
        }
      }
    }
  }

  for (auto it = m_entities.begin(); it != m_entities.end(); ++it)
  {
    int px = (*it)->x;
    int py = (*it)->y;

    if (px >= m_scrollX && py >= m_scrollY && px < m_scrollXMax && py < m_scrollYMax)
    {
      int posX = TILE_W*(px - m_scrollX) + m_editArea.left;
      int posY = TILE_H*(py - m_scrollY) + m_editArea.top;

      sf::RectangleShape tmpRect;
      tmpRect.setPosition(posX, posY);
      tmpRect.setSize(sf::Vector2f(TILE_W, TILE_H));
      tmpRect.setFillColor(sf::Color::Blue);
      tmpRect.setOutlineColor(sf::Color::Transparent);
      m_window.draw(tmpRect);
    }
  }

  for (int y = 0; y < m_editArea.height / TILE_H; y++)
  {
    for (int x = 0; x < m_editArea.width / TILE_W; x++)
    {
      int px = x + m_scrollX;
      int py = y + m_scrollY;

      if (px >= m_mapW || py >= m_mapH)
      {
        int posX = (px - m_scrollX) * TILE_W + m_editArea.left + 1;
        int posY = (py - m_scrollY) * TILE_H + m_editArea.top + 1;

        if (m_editArea.contains(posX, posY))
        {
          rect.setPosition(posX, posY);
          rect.setSize(sf::Vector2f(TILE_W-2, TILE_H-2));
          rect.setFillColor(sf::Color(64, 64, 64));
          rect.setOutlineColor(sf::Color(64, 64, 64));
          m_window.draw(rect);
        }
      }
    }
  }

  int mouseX = sf::Mouse::getPosition(m_window).x;
  int mouseY = sf::Mouse::getPosition(m_window).y;

  if (m_editArea.contains(mouseX, mouseY))
  {
    mouseX /= TILE_W;
    mouseY /= TILE_H;

    rect.setPosition(mouseX * TILE_W, mouseY * TILE_H);
    rect.setSize(sf::Vector2f(TILE_W, TILE_H));
    rect.setFillColor(sf::Color::Transparent);
    rect.setOutlineColor(sf::Color::Red);
    m_window.draw(rect);
  }


}

void Editor::buildTileParts()
{
  int numTilesX = m_tileset->getSize().x / TILE_W;
  int numTilesY = m_tileset->getSize().y / TILE_H;

  for (int y = 0; y < numTilesY; y++)
  {
    for (int x = 0; x < numTilesX; x++)
    {
      TilePart part = { x, y };
      m_tileParts.push_back(part);
    }
  }

}

const Editor::TilePart* Editor::getTileAt(int x, int y, int layer) const
{
  if (x < 0 || y < 0 || x >= m_mapW || y >= m_mapH)
    return 0;

  int index = y * m_mapW + x;
  if (index >= 0 && index < getNumberOfTiles())
  {
    return &m_tiles[layer][index];
  }
  return 0;
}

void Editor::updateTile(int x, int y)
{
  if (x < 0 || y < 0 || x >= m_mapW || y >= m_mapH)
    return;

  int index = y * m_mapW + x;
  if (index >= 0 && index < getNumberOfTiles())
  {
    m_tiles[m_currentLayer][index].tileX = m_currentTile.tileX;
    m_tiles[m_currentLayer][index].tileY = m_currentTile.tileY;
  }
}

void Editor::doFloodFill(int px, int py)
{
  const TilePart* originTile = getTileAt(px, py, m_currentLayer);

  if (!originTile)
    return;

  int* floodMap = new int[m_mapW * m_mapH]();
  for (int y = 0; y < m_mapH; y++)
  {
    for (int x = 0; x < m_mapW; x++)
    {
      int index = y * m_mapW + x;

      const TilePart* t = getTileAt(x, y, m_currentLayer);
      if (t->tileX == originTile->tileX && t->tileY == originTile->tileY)
      {

        floodMap[index] = 1;
      }
      else
      {
        floodMap[index] = 0;
      }
    }
  }

  floodfill(floodMap, m_mapW, m_mapH, px, py, 2);

  for (int y = 0; y < m_mapH; y++)
  {
    for (int x = 0; x < m_mapW; x++)
    {
      int index = y * m_mapW + x;

      if (floodMap[index] == 2)
      {
        updateTile(x, y);
      }
    }
  }
}

void Editor::resizeMap(int width, int height)
{
  TRACE("Changing map size from (%d %d) to (%d %d)", m_mapW, m_mapH, width, height);

  TilePart* newTiles[MAX_LAYERS];
  for (int i = 0; i < MAX_LAYERS; i++)
  {
    newTiles[i] = new TilePart[width * height]();
  }

  for (int i = 0; i < MAX_LAYERS; i++)
  {
    for (int j = 0; j < width * height; j++)
    {
      newTiles[i][j].tileX = 0;//m_currentTile.tileX;
      newTiles[i][j].tileY = 0;//m_currentTile.tileY;
    }
  }

  for (int i = 0; i < MAX_LAYERS; i++)
  {
    for (int y = 0; y < m_mapH; y++)
    {
      for (int x = 0; x < m_mapW; x++)
      {
        const TilePart* tp = getTileAt(x, y, i);

        if (x < width && y < height)
        {
          // Index for new map.
          int index = y * width + x;
          newTiles[i][index].tileX = tp->tileX;
          newTiles[i][index].tileY = tp->tileY;
        }
      }
    }
  }

  for (int i = 0; i < MAX_LAYERS; i++)
  {
    delete[] m_tiles[i];
    m_tiles[i] = newTiles[i];
  }

  m_mapW = width;
  m_mapH = height;
}

void Editor::handleCarriageReturn()
{
  if (m_textInputState == TEXT_INPUT_RESIZE)
  {
    std::istringstream ss(m_currentInput);
    int w, h;
    ss >> w >> h;
    resizeMap(w, h);
  }
}

void Editor::setTextInputState(TextInputState newState)
{
  std::string oldStateString = textInputStateToString();

  m_textInputState = newState;

  std::string newStateString = textInputStateToString();

  TRACE("Changing textInputState from %s to %s", oldStateString.c_str(), newStateString.c_str());
}

std::string Editor::textInputStateToString() const
{
  switch (m_textInputState)
  {
  case TEXT_INPUT_NONE:
    return "TEXT_INPUT_NONE";
  case TEXT_INPUT_RESIZE:
    return "TEXT_INPUT_RESIZE";
  case TEXT_INPUT_SAVE_MAP:
    return "TEXT_INPUT_SAVE_MAP";
  case TEXT_INPUT_LOAD_MAP:
    return "TEXT_INPUT_LOAD_MAP";
  case TEXT_INPUT_SELECT_MUSIC:
    return "TEXT_INPUT_SELECT_MUSIC";
  default:
    return "<Unknown textInputState>";
  }
}

const Entity* Editor::getEntityAt(int x, int y) const
{
  for (auto it = m_entities.begin(); it != m_entities.end(); ++it)
  {
    if ((int)(*it)->x == x && (int)(*it)->y == y)
    {
      return *it;
    }
  }

  return 0;
}
