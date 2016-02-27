#ifndef XMLHELPERS_H_
#define XMLHELPERS_H_

#include <sstream>
#include "../dep/tinyxml2.h"

template <typename T>
struct xml_parse_attribute
{
  static T parse(const tinyxml2::XMLElement* element, const std::string& attrName)
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
};

template <>
struct xml_parse_attribute<std::string>
{
  static std::string parse(const tinyxml2::XMLElement* element, const std::string& attrName)
  {
    const tinyxml2::XMLAttribute* attribute = element->FindAttribute(attrName.c_str());

    if (attribute)
    {
      return attribute->Value();
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

inline bool valid_text_element(const tinyxml2::XMLElement* element)
{
  return element && element->GetText();
}

#endif /* XMLHELPERS_H_ */
