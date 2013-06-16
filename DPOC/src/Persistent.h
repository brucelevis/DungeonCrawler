#ifndef PERSISTENT_H
#define PERSISTENT_H

// Global variables, local variables, global toggles, local toggles.

#include <string>
#include <map>

namespace persistent
{
  template <typename T, typename What>
  class Persistent
  {
  public:
    static What& instance()
    {
      static What what;
      return what;
    }

    bool isSet(const std::string& key) const
    {
      return m_storage.count(key) > 0;
    }

    T get(const std::string& key) const
    {
      typename std::map<std::string, T>::iterator it = m_storage.find(key);
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
  protected:
    Persistent() {}
  private:
    std::map<std::string, T> m_storage;
  };

  template <typename T>
  class Global : public Persistent<T, Global<T> >
  {
    friend class Persistent<T, Global<T> >;
    Global() {}
  };

  template <typename T>
  class Local : public Persistent<T, Local<T> >
  {
    friend class Persistent<T, Local<T> >;
    Local() {}
  };
}

#endif
