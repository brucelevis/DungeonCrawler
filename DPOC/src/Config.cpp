#include <map>
#include <string>

#include "Config.h"

#include "../dep/tinyxml2.h"

using namespace tinyxml2;

namespace config
{

  static std::map<std::string, std::string> CONFIG;

  void load_config()
  {
    static const std::string conf = "Resources/Config.xml";

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
}
