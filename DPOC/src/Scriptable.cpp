#include "LuaBindings.h"
#include "Scriptable.h"

Scriptable::Scriptable()
{
  register_lua_bindings(m_luaState);

  m_luaState.register_function("_set_integer", &Scriptable::setIntegerArg);
  m_luaState.register_function("_set_float",   &Scriptable::setFloatArg);
  m_luaState.register_function("_set_double",  &Scriptable::setDoubleArg);
  m_luaState.register_function("_set_string",  &Scriptable::setStringArg);
  m_luaState.register_function("_set_pointer", &Scriptable::setPointerArg);
}

void Scriptable::setIntegerArg(const char* argName, int value)
{
  m_luaState.register_global(argName, value);
  invalidate();
}

void Scriptable::setFloatArg(const char* argName, float value)
{
  m_luaState.register_global(argName, value);
  invalidate();
}

void Scriptable::setDoubleArg(const char* argName, double value)
{
  m_luaState.register_global(argName, value);
  invalidate();
}

void Scriptable::setStringArg(const char* argName, const char* argValue)
{
  m_luaState.register_global(argName, argValue);
  invalidate();
}

void Scriptable::setPointerArg(const char* argName, void* argValue)
{
  m_luaState.register_global(argName, argValue);
  invalidate();
}
