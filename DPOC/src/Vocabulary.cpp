#include <unordered_map>

#include "Utility.h"
#include "logger.h"

#include "Config.h"
#include "Vocabulary.h"

#include "../dep/tinyxml2.h"
using namespace tinyxml2;

namespace
{
  struct Term
  {
    std::string name;
    std::string shortName;
    std::string midName;
  };

  std::unordered_map<std::string, Term> vocabulary_terms;
}

const std::string& vocab(const std::string& termName)
{
  auto it = vocabulary_terms.find(termName);

  if (it != vocabulary_terms.end())
  {
    return it->second.name;
  }

  throw std::runtime_error("No term: " + termName + " found!");
}

std::string vocab_upcase(const std::string& termName)
{
  auto it = vocabulary_terms.find(termName);

  if (it != vocabulary_terms.end())
  {
    return capitalize(it->second.name);
  }

  throw std::runtime_error("No term: " + termName + " found!");
}

const std::string& vocab_mid(const std::string& termName)
{
  auto it = vocabulary_terms.find(termName);

  if (it != vocabulary_terms.end())
  {
    return it->second.midName;
  }

  throw std::runtime_error("No term: " + termName + " found!");
}

const std::string& vocab_short(const std::string& termName)
{
  auto it = vocabulary_terms.find(termName);

  if (it != vocabulary_terms.end())
  {
    return it->second.shortName;
  }

  throw std::runtime_error("No term: " + termName + " found!");
}

void load_vocabulary()
{
  static const std::string database = config::res_path("Vocabulary.xml");

  TRACE("Loading Vocabulary file %s", database.c_str());

  XMLDocument doc;
  if (doc.LoadFile(database.c_str()) != 0)
  {
    TRACE("Unable to open vocabulary database %s (%s)!", database.c_str(), doc.GetErrorStr1());

    throw std::runtime_error("Unable to open vocabulary database " + database);
  }

  if (const XMLElement* root = doc.FirstChildElement("vocabulary"))
  {
    for (const XMLElement* element = root->FirstChildElement("term"); element; element = element->NextSiblingElement("term"))
    {
      std::string term = element->FindAttribute("name")->Value();

      std::string name = element->FindAttribute("base")->Value();
      std::string shortName = element->FindAttribute("short")->Value();
      std::string mid;

      if (const XMLAttribute* midAttr = element->FindAttribute("mid"))
      {
        mid = midAttr->Value();
      }

      TRACE("Term %s: long='%s', short='%s'", term.c_str(), name.c_str(), shortName.c_str());

      vocabulary_terms[term] = { name, shortName, mid };
    }
  }
}
