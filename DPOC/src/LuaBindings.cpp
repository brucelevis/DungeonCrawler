#include <vector>

#include "Config.h"
#include "logger.h"

#include "Message.h"
#include "Character.h"
#include "Player.h"
#include "PlayerCharacter.h"
#include "Entity.h"
#include "SceneManager.h"
#include "Sound.h"
#include "Utility.h"

#include "Lua.h"
#include "LuaBindings.h"

void run_lua_script(const std::string& script)
{
  if (!lua::run(config::res_path(script)))
  {
    TRACE("Unable to run script %s: %s", script.c_str(), lua::error().c_str());
  }
}

void register_lua_bindings()
{
  lua::reg()
    // Misc functions
    ("play_sound", [](std::string sound) { play_sound(sound); })
    ("shake_screen", [](int duration, int power) { SceneManager::instance().shakeScreen(duration, power, power); })
    ("message", [](std::string msg) { show_message(msg.c_str()); })

    // Character functions
    ("afflict_status", [](Character* chr, std::string status, int duration) {chr->afflictStatus(status, duration); })
    ("cure_status", [](Character* chr, std::string status) { chr->cureStatus(status); })
    ("check_vs_luck", check_vs_luck)
    ("get_attribute", [](Character* chr, std::string attribute) { return chr->computeCurrentAttribute(attribute); })
    ("deal_damage", [](Character* chr, int amount) { chr->takeDamage("hp", amount); })
    ("get_character_name", &Character::getName)

    // Player functions
    ("get_party_size", []() { return get_player()->getParty().size(); })
    ("get_party_member", [](int index) { return get_player()->getParty().at(index); });
}
