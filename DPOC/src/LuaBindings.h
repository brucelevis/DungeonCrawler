#ifndef LUA_BINDINGS_H
#define LUA_BINDINGS_H

#include <string>
#include "Lua.h"

void register_lua_bindings(lua::LuaEnv& luaState);
void run_lua_script(lua::LuaEnv& luaState, const std::string& script);
void run_lua_string(lua::LuaEnv& luaState, const std::string& line);

#endif
