#ifndef LUA_BINDINGS_H
#define LUA_BINDINGS_H

#include <string>

void register_lua_bindings();
void run_lua_script(const std::string& script);

#endif
