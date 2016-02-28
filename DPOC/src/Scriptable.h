#ifndef SCRIPTABLE_H_
#define SCRIPTABLE_H_

#include <string>
#include "Lua.h"

struct Scriptable
{
  Scriptable();
  virtual ~Scriptable() {}
//private:
  void setIntegerArg(const char* argName, int value);
  void setFloatArg(const char* argName, float value);
  void setDoubleArg(const char* argName, double value);
  void setStringArg(const char* argName, const char* argValue);
  void setPointerArg(const char* argName, void* argValue);
protected:
  virtual void invalidate() {}
protected:
  mutable lua::LuaEnv m_luaState;
};

#endif /* SCRIPTABLE_H_ */
