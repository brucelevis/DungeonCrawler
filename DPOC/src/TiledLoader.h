#ifndef TILED_LOADER_H
#define TILED_LOADER_H

#include <map>
#include <string>
#include <vector>

#include "../dep/tinyxml2.h"

class TiledLoader
{
public:
  struct Layer
  {
    int width, height;
    std::vector<int> tiles;
    std::map<std::string, std::string> properties;
  };

  struct Object
  {
    std::string name;
    std::string type;
    int x, y;
    int width, height;
    std::map<std::string, std::string> properties;
    int tileId; // 0 if not tile object.
  };

  /**
   * How do we know which Tileset belong to which layer?
   */
  struct Tileset
  {
    int tileW, tileH;
    int startTileIndex; // Subtract this from tilenumber to get real index.
    std::string tilesetSource;
    bool useColorKey;
    int colorKey_r; // For transparency.
    int colorKey_g;
    int colorKey_b;
  };
public:
  TiledLoader();
  bool loadFromFile(const std::string& fileName);

  inline int getWidth() const { return m_width; }
  inline int getHeight() const { return m_height; }
  inline int getTileWidth() const { return m_tileW; }
  inline int getTileHeight() const { return m_tileH; }

  inline size_t getNumberOfObjects() const { return m_objects.size(); }
  inline std::string getObjectName(size_t index) const { return m_objects[index].name; }
  inline std::string getObjectType(size_t index) const { return m_objects[index].type; }
  inline int getObjectX(size_t index) const { return m_objects[index].x; }
  inline int getObjectY(size_t index) const { return m_objects[index].y; }
  inline int getObjectWidth(size_t index) const { return m_objects[index].width; }
  inline int getObjectHeight(size_t index) const { return m_objects[index].height; }
  std::string getObjectProperty(size_t index, const std::string& property) const;

  std::vector<std::string> getLayers() const;
  std::vector<int> getLayerTileIndices(const std::string& id) const;
  std::string getLayerProperty(const std::string& ident, const std::string& property) const;

  std::vector<std::string> getTilesets() const;
  int getTilesetTileWidth(const std::string& id) const;
  int getTilesetTileHeight(const std::string& id) const;
  int getTilesetStartTileIndex(const std::string& id) const;
  std::string getTilesetSource(const std::string& id) const;
  bool tilesetUsesColorKey(const std::string& id) const;
  void getTilesetColorKeyRGB(const std::string& id, int& r, int& g, int& b);

  std::string getProperty(const std::string& property) const;

  const Tileset* getTileset(const std::string& ident) const;
  const Layer* getLayer(const std::string& ident) const;
  inline const Object* getObject(size_t index) const { return &m_objects[index]; }


  const Tileset* findTilesetMatchingTileIndex(int tileIndex) const;
private:
  void setMapAttributes(const tinyxml2::XMLElement* mapElement);

  void parseProperties(const tinyxml2::XMLElement* element, std::map<std::string, std::string>& properties);
  void parseTileset(const tinyxml2::XMLElement* element);
  void parseObjectGroup(const tinyxml2::XMLElement* element);
  void parseLayer(const tinyxml2::XMLElement* element);

  std::vector<int> parseCsv(const std::string& csv) const;
private:
  std::string m_orientation;
  int m_width;
  int m_height;
  int m_tileW;
  int m_tileH;

  std::map<std::string, std::string> m_properties;
  std::map<std::string, Layer> m_layers;
  std::map<std::string, Tileset> m_tilesets;
  std::vector<Object> m_objects;
};

#endif
