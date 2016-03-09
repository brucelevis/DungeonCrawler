#ifndef PERSISTENT_H
#define PERSISTENT_H

// Global variables, local variables, global toggles, local toggles.

#include <sstream>
#include <string>
#include <unordered_map>

#include "Utility.h"
#include "logger.h"

class Persistent
{
public:
  static Persistent& instance()
  {
    static Persistent what;
    return what;
  }

  bool isSet(const std::string& key) const
  {
    return m_storage.count(key) > 0;
  }

  std::string get(const std::string& key) const
  {
    auto it = m_storage.find(key);
    if (it != m_storage.end())
    {
      return it->second;
    }
    return "";
  }

  template <typename T>
  T getAs(const std::string& key) const
  {
    return fromString<T>(get(key));
  }

  void set(const std::string& key, const std::string& value)
  {
    m_storage[key] = value;
  }

  template <typename T>
  void set(const std::string& key, const T& value)
  {
    m_storage[key] = toString(value);
  }

  std::string xmlDump() const
  {
    std::string xml = "<persistents>\n";

    for (auto it = m_storage.begin(); it != m_storage.end(); ++it)
    {
      std::ostringstream ss;
      ss << " <data key=\"" << it->first << "\" value=\"" << it->second << "\" />\n";
      xml += ss.str();
    }

    xml += "</persistents>\n";
    return xml;
  }

  void clear()
  {
    m_storage.clear();
  }
protected:
  Persistent() {}
private:
  std::unordered_map<std::string, std::string> m_storage;
};

template <typename T>
inline T global(const std::string& name)
{
  return Persistent::instance().getAs<T>(name);
}

template <typename T>
inline void set_global(const std::string& name, const T& value)
{
  Persistent::instance().set(name, value);
}

#endif
