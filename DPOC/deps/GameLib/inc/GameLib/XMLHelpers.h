#ifndef GAMELIB_XMLHELPERS_H_
#define GAMELIB_XMLHELPERS_H_

#include <sstream>
#include <string>
#include <memory>
#include <stdexcept>

#include <tinyxml2.h>

namespace gamelib
{
  template <typename T>
  struct xml_parse
  {
    static T attribute(const tinyxml2::XMLElement* element, const std::string& attrName)
    {
      const tinyxml2::XMLAttribute* attribute = element->FindAttribute(attrName.c_str());

      if (attribute)
      {
        T t;

        std::istringstream ss ( attribute->Value() );
        ss >> t;

        return t;
      }

      return T();
    }

    static T text(const tinyxml2::XMLElement* element)
    {
      if (element->GetText())
      {
        std::string text = element->GetText();

        T t;

        std::istringstream ss ( text );
        ss >> t;

        return t;
      }

      return T();
    }
  };

  template <>
  struct xml_parse<std::string>
  {
    static std::string attribute(const tinyxml2::XMLElement* element, const std::string& attrName)
    {
      const tinyxml2::XMLAttribute* attribute = element->FindAttribute(attrName.c_str());

      if (attribute)
      {
        return attribute->Value();
      }

      return "";
    }

    static std::string text(const tinyxml2::XMLElement* element)
    {
      if (element->GetText())
      {
        return element->GetText();
      }

      return "";
    }
  };

  template <typename Func>
  void xml_for_each(const tinyxml2::XMLElement* startElement, Func func)
  {
    for (const tinyxml2::XMLElement* element = startElement->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      func(element);
    }
  }

  template <typename T>
  std::string xml_make_tag(const std::string& name, const T& attribute)
  {
    return "<" + name + ">" + to_string(attribute) + "</" + name + ">";
  }

  typedef std::unique_ptr<tinyxml2::XMLDocument> XmlDocumentPtr;

  inline XmlDocumentPtr xml_open_document(const std::string& filename)
  {
    XmlDocumentPtr doc { new tinyxml2::XMLDocument };

    if (doc->LoadFile(filename.c_str()) != 0)
    {
      throw std::runtime_error("Unable to open XML file: " + filename);
    }

    return std::move(doc);
  }
}

#endif /* XMLHELPERS_H_ */
