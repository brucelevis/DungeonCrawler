#ifndef PERSISTENT_H
#define PERSISTENT_H

// Global variables, local variables, global toggles, local toggles.

#include <sstream>
#include <string>
#include <map>

#include "logger.h"

template <typename T>
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

  T get(const std::string& key) const
  {
    auto it = m_storage.find(key);
    if (it != m_storage.end())
    {
      return it->second;
    }
    return T();
  }

  void set(const std::string& key, T value)
  {
    m_storage[key] = value;
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
  std::map<std::string, T> m_storage;
};

template <typename T>
inline T global(const std::string& name)
{
  return Persistent<T>::instance().get(name);
}

#endif
