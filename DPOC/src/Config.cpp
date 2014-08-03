#include <map>
#include <string>

#include "Config.h"

#include "../dep/tinyxml2.h"

using namespace tinyxml2;

namespace config
{

  int TILE_W = 16;
  int TILE_H = 16;

  static std::map<std::string, std::string> CONFIG;

  void load_config()
  {
    static const std::string conf = RESOURCE_DIR + "Config.xml";

    XMLDocument doc;
    doc.LoadFile(conf.c_str());

    const XMLElement* root = doc.FirstChildElement("config");
    for (const XMLElement* element = root->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      std::string name = element->Name();
      std::string value = element->GetText();

      CONFIG[name] = value;
    }
  }

  std::string get(const std::string& key)
  {
    return CONFIG[key];
  }

  void set(const std::string& key, const std::string& value)
  {
    CONFIG[key] = value;
  }

  bool isSet(const std::string& key)
  {
    return CONFIG.count(key);
  }
}
