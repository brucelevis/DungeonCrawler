#include <iostream>
#include <sstream>
#include <cstdlib>
#include <iomanip>

#include "logger.h"

#include "TiledLoader.h"
#include "../dep/tinyxml2.h"

using namespace tinyxml2;

TiledLoader::TiledLoader()
 : m_width(0),
   m_height(0),
   m_tileW(0),
   m_tileH(0)
{

}

bool TiledLoader::loadFromFile(const std::string& fileName)
{
  TRACE("Loading tiled XML map: %s", fileName.c_str());

  XMLDocument doc;
  doc.LoadFile(fileName.c_str());

  XMLElement* mapElement = doc.FirstChildElement("map");
  XMLElement* element = 0;

  setMapAttributes(mapElement);

  for (element = mapElement->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    std::string elementName = element->Name();

    if (elementName == "properties")
    {
      parseProperties(element, m_properties);
    }
    else if (elementName == "tileset")
    {
      parseTileset(element);
    }
    else if (elementName == "layer")
    {
      parseLayer(element);
    }
    else if (elementName == "objectgroup")
    {
      parseObjectGroup(element);
    }
  }


  return true;
}

void TiledLoader::setMapAttributes(const tinyxml2::XMLElement* mapElement)
{
 /* orientation: Map orientation. Tiled supports "orthogonal" and "isometric" at the moment.
  * width: The map width in tiles.
  * height: The map height in tiles.
  * tilewidth: The width of a tile.
  * tileheight: The height of a tile.
  */

  const XMLAttribute* orientAttrib = mapElement->FindAttribute("orientation");
  const XMLAttribute* widthAttrib = mapElement->FindAttribute("width");
  const XMLAttribute* heightAttrib = mapElement->FindAttribute("height");
  const XMLAttribute* tileWAttrib = mapElement->FindAttribute("tilewidth");
  const XMLAttribute* tileHAttrib = mapElement->FindAttribute("tileheight");

  if (orientAttrib) m_orientation = orientAttrib->Value();
  if (widthAttrib) m_width = widthAttrib->IntValue();
  if (heightAttrib) m_height = heightAttrib->IntValue();
  if (tileWAttrib) m_tileW = tileWAttrib->IntValue();
  if (tileHAttrib) m_tileH = tileHAttrib->IntValue();
}

void TiledLoader::parseProperties(const tinyxml2::XMLElement* element, std::map<std::string, std::string>& properties)
{
  const XMLElement* propElement = 0;

  for (propElement = element->FirstChildElement(); propElement; propElement = propElement->NextSiblingElement())
  {
    const XMLAttribute* nameAttrib = propElement->FindAttribute("name");
    const XMLAttribute* valueAttrib = propElement->FindAttribute("value");

    if (valueAttrib)
    {
      std::ostringstream ss;
      ss << "* Reading property " << nameAttrib->Value() << " = " << valueAttrib->Value();
      TRACE("%s", ss.str().c_str());

      properties[nameAttrib->Value()] = valueAttrib->Value();
    }
    else
    {
      // If no value attribute exist on the the property, get the containing text.
      properties[nameAttrib->Value()] = propElement->GetText();

      TRACE("* Reading property: %s. Text=%s", nameAttrib->Value(), propElement->GetText());
    }
  }
}

void TiledLoader::parseTileset(const tinyxml2::XMLElement* element)
{
  TRACE("parseTileset");

  const XMLAttribute* firstgidAttrib = element->FindAttribute("firstgid");
  const XMLAttribute* nameAttrib = element->FindAttribute("name");
  const XMLAttribute* tileWAttrib = element->FindAttribute("tilewidth");
  const XMLAttribute* tileHAttrib = element->FindAttribute("tileheight");

  const XMLElement* imageElement = element->FirstChildElement("image");

  const XMLAttribute* sourceAttrib = imageElement->FindAttribute("source");
  const XMLAttribute* transAttrib = imageElement->FindAttribute("trans");

  Tileset tileset;
  tileset.startTileIndex = firstgidAttrib->IntValue();
  tileset.tileW = tileWAttrib->IntValue();
  tileset.tileH = tileHAttrib->IntValue();
  tileset.tilesetSource = sourceAttrib->Value();

  if (transAttrib)
  {
    tileset.useColorKey = true;

    std::string trans = transAttrib->Value();
    std::string r("xx");
    std::string g("xx");
    std::string b("xx");

    r[0] = trans[0];
    r[1] = trans[1];
    g[0] = trans[2];
    g[1] = trans[3];
    b[0] = trans[4];
    b[1] = trans[5];

    r = "0x" + r;
    g = "0x" + g;
    b = "0x" + b;

    std::istringstream ss(r + " " + g + " " + b);
    ss >> std::hex >> tileset.colorKey_r >> tileset.colorKey_g >> tileset.colorKey_b >> std::dec;

    TRACE("colorKey_r=%d", tileset.colorKey_r);
    TRACE("colorKey_g=%d", tileset.colorKey_g);
    TRACE("colorKey_b=%d", tileset.colorKey_b);
  }
  else
  {
    tileset.useColorKey = false;
    tileset.colorKey_r = tileset.colorKey_g = tileset.colorKey_b = 0;
  }

  std::ostringstream ss;
  ss << "* Adding new tileset: " << std::endl
     << " name: " << nameAttrib->Value() << std::endl
     << " tileW: " << tileset.tileW << std::endl
     << " tileH: " << tileset.tileH << std::endl
     << " startTileIndex: " << tileset.startTileIndex << std::endl
     << " tilesetSource: " << tileset.tilesetSource << std::endl
     << " useColorKey: " << tileset.useColorKey << std::endl
     << " colorKey: 0x" << std::hex << tileset.colorKey_r << tileset.colorKey_g << tileset.colorKey_b << std::dec << std::endl;
  TRACE("%s", ss.str().c_str());

  m_tilesets[nameAttrib->Value()] = tileset;
}

void TiledLoader::parseObjectGroup(const tinyxml2::XMLElement* element)
{
  const XMLElement* objectElement = 0;

  for (objectElement = element->FirstChildElement("object"); objectElement; objectElement = objectElement->NextSiblingElement())
  {
    const XMLAttribute* nameAttrib = objectElement->FindAttribute("name");
    const XMLAttribute* typeAttrib = objectElement->FindAttribute("type");
    const XMLAttribute* xAttrib = objectElement->FindAttribute("x");
    const XMLAttribute* yAttrib = objectElement->FindAttribute("y");
    const XMLAttribute* widthAttrib = objectElement->FindAttribute("width");
    const XMLAttribute* heightAttrib = objectElement->FindAttribute("height");
    const XMLAttribute* gidAttrib = objectElement->FindAttribute("gid");

    Object object;
    object.name = nameAttrib ? nameAttrib->Value() : "";
    object.type = typeAttrib ? typeAttrib->Value() : "";
    object.x = xAttrib->IntValue();
    object.y = yAttrib->IntValue();
    object.width = widthAttrib ? widthAttrib->IntValue() : -1;
    object.height = heightAttrib ? heightAttrib->IntValue() : -1;
    object.tileId = gidAttrib ? gidAttrib->IntValue() : 0;

    if (gidAttrib)
    {
      // Special case since tiled places tile objects wrong for some
      // reason.
      object.y -= getTileHeight();
    }

    std::ostringstream ss;
    ss << "* Adding object new object: " << std::endl
       << " name:   " << object.name     << std::endl
       << " type:   " << object.type     << std::endl
       << " x:      " << object.x        << std::endl
       << " y:      " << object.y        << std::endl
       << " width:  " << object.width    << std::endl
       << " height: " << object.height   << std::endl
       << " tileId: " << object.tileId   << std::endl;
    TRACE("%s", ss.str().c_str());

    const XMLElement* propElement = objectElement->FirstChildElement("properties");
    if (propElement)
    {
      parseProperties(propElement, object.properties);
    }

    m_objects.push_back(object);
  }

}

void TiledLoader::parseLayer(const tinyxml2::XMLElement* element)
{
  const XMLAttribute* nameAttrib = element->FindAttribute("name");
  const XMLAttribute* widthAttrib = element->FindAttribute("width");
  const XMLAttribute* heightAttrib = element->FindAttribute("height");

  Layer layer;
  layer.width = widthAttrib->IntValue();
  layer.height = heightAttrib->IntValue();

  const XMLElement* dataElement = element->FirstChildElement("data");
  const XMLAttribute* encodingAttrib = dataElement->FindAttribute("encoding");

  const XMLElement* propertyElement = element->FirstChildElement("properties");
  if (propertyElement)
  {
    parseProperties(propertyElement, layer.properties);
  }

  if (std::string(encodingAttrib->Value()) == "csv")
  {
    layer.tiles = parseCsv(dataElement->GetText());
  }
  else
  {
    std::cerr << "* Not supported encoding: " << encodingAttrib->Value() << std::endl;
  }

  m_layers[nameAttrib->Value()] = layer;

  std::ostringstream ss;
  ss << "* Adding new layer: " << std::endl
     << " name:   " << nameAttrib->Value() << std::endl
     << " width:  " << layer.width << std::endl
     << " height: " << layer.height << std::endl;
  TRACE("%s", ss.str().c_str());
}

std::vector<int> TiledLoader::parseCsv(const std::string& csv) const
{
  std::vector<int> tiles;

  std::stringstream csvStream(csv);

  std::string line;
  while (std::getline(csvStream, line))
  {
    std::stringstream lineStream(line);

    std::string element;
    while (std::getline(lineStream, element, ','))
    {
      int value = atoi(element.c_str());
      tiles.push_back(value);
    }
  }

  TRACE("* parseCsv: Read %d tile numbers.", tiles.size());

  return tiles;
}

std::vector<std::string> TiledLoader::getLayers() const
{
  std::vector<std::string> layers;
  for (std::map<std::string, Layer>::const_iterator iter = m_layers.begin();
       iter != m_layers.end();
       ++iter)
  {
    layers.push_back(iter->first);
  }
  return layers;
}

std::vector<std::string> TiledLoader::getTilesets() const
{
  std::vector<std::string> tilesets;
  for (std::map<std::string, Tileset>::const_iterator iter = m_tilesets.begin();
       iter != m_tilesets.end();
       ++iter)
  {
    tilesets.push_back(iter->first);
  }
  return tilesets;
}

std::string TiledLoader::getProperty(const std::string& property) const
{
  std::map<std::string, std::string>::const_iterator iter = m_properties.find(property);
  if (iter != m_properties.end())
  {
    return iter->second;
  }
  return "";
}

std::string TiledLoader::getObjectProperty(size_t index, const std::string& property) const
{
  const Object* o = &m_objects[index];

  std::map<std::string, std::string>::const_iterator iter = o->properties.find(property);

  if (iter != o->properties.end())
  {
    return iter->second;
  }

  return "";
}

const TiledLoader::Tileset* TiledLoader::getTileset(const std::string& ident) const
{
  std::map<std::string, Tileset>::const_iterator iter = m_tilesets.find(ident);
  if (iter != m_tilesets.end())
  {
    return &iter->second;
  }
  return 0;
}

const TiledLoader::Layer* TiledLoader::getLayer(const std::string& ident) const
{
  std::map<std::string, Layer>::const_iterator iter = m_layers.find(ident);
  if (iter != m_layers.end())
  {
    return &iter->second;
  }
  return 0;
}

int TiledLoader::getTilesetTileWidth(const std::string& id) const
{
  const Tileset* tileset = getTileset(id);
  if (!tileset)
    return -1;

  return tileset->tileW;
}

int TiledLoader::getTilesetTileHeight(const std::string& id) const
{
  const Tileset* tileset = getTileset(id);
  if (!tileset)
    return -1;

  return tileset->tileH;
}

int TiledLoader::getTilesetStartTileIndex(const std::string& id) const
{
  const Tileset* tileset = getTileset(id);
  if (!tileset)
    return -1;

  return tileset->startTileIndex;
}

std::string TiledLoader::getTilesetSource(const std::string& id) const
{
  const Tileset* tileset = getTileset(id);
  if (!tileset)
    return "";

  return tileset->tilesetSource;
}

bool TiledLoader::tilesetUsesColorKey(const std::string& id) const
{
  const Tileset* tileset = getTileset(id);
  if (!tileset)
    return false;

  return tileset->useColorKey;
}

void TiledLoader::getTilesetColorKeyRGB(const std::string& id, int& r, int& g, int& b)
{
  const Tileset* tileset = getTileset(id);
  if (!tileset)
    return;

  r = tileset->colorKey_r;
  g = tileset->colorKey_g;
  b = tileset->colorKey_b;
}

std::vector<int> TiledLoader::getLayerTileIndices(const std::string& id) const
{
  const Layer* layer = getLayer(id);
  if (!layer)
    return std::vector<int>();

  return layer->tiles;
}

std::string TiledLoader::getLayerProperty(const std::string& ident, const std::string& property) const
{
  const Layer* layer = getLayer(ident);
  if (!layer)
    return "";

  std::map<std::string, std::string>::const_iterator iter = layer->properties.find(property);
  if (iter != layer->properties.end())
  {
    return iter->second;
  }
  return "";
}
