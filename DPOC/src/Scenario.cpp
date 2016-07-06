#include <string>
#include <stdexcept>

#include "logger.h"
#include "Scenario.h"

#include "../dep/tinyxml2.h"
using namespace tinyxml2;

Scenario::Scenario()
  : m_useCharGen(false),
    m_name("DPOC")
{
  XMLDocument doc;
  if (doc.LoadFile("Scenario.xml") != 0)
  {
    throw std::runtime_error{"Fatal error: no Scenario.xml file found!"};
  }

  TRACE("Loaded scenario file");

  XMLElement* root = doc.FirstChildElement("scenario");
  for (XMLElement* element = root->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    std::string tagName = element->Name();

    if (tagName == "createParty")
    {
      m_useCharGen = std::string(element->GetText()) == "true";
    }
    else if (tagName == "name")
    {
      m_name = element->GetText();
    }
  }

  TRACE(" - m_gameName   = %s", m_name.c_str());
  TRACE(" - m_useCharGen = %s", m_useCharGen ? "true" : "false");
}

Scenario& Scenario::instance()
{
  static Scenario scenario;
  return scenario;
}
