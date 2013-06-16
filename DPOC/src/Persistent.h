#ifndef PERSISTENT_H
#define PERSISTENT_H

// Global variables, local variables, global toggles, local toggles.

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
      TRACE("GET %s = %d", key.c_str(), it->second);

      return it->second;
    }
    return T();
  }

  void set(const std::string& key, T value)
  {
    TRACE("SET %s = %d", key.c_str(), value);

    m_storage[key] = value;
  }
protected:
  Persistent() {}
private:
  std::map<std::string, T> m_storage;
};

#endif
