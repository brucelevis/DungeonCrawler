#include <vector>

#include "Cache.h"

#include "Config.h"
#include "logger.h"

#include "Encounter.h"
#include "Message.h"
#include "Character.h"
#include "Player.h"
#include "PlayerCharacter.h"
#include "Entity.h"
#include "SceneManager.h"
#include "Sound.h"
#include "Utility.h"
#include "Persistent.h"
#include "SkillTrainer.h"
#include "Frame.h"
#include "draw_text.h"

#include "Lua.h"
#include "LuaBindings.h"

namespace
{
  ////////////////////////////////////////////////////////////////////////////
  // Music functions
  ////////////////////////////////////////////////////////////////////////////
  sf::Music* lua_loadMusic(const std::string& fileName)
  {
    sf::Music* music = new sf::Music;
    music->openFromFile( config::res_path(fileName) );
    return music;
  }

  void lua_freeMusic(sf::Music* music)
  {
    if (music->getStatus() == sf::Music::Playing)
    {
      music->stop();
    }

    delete music;
  }

  void lua_playMusic(sf::Music* music)
  {
    music->play();
  }

  void lua_stopMusic(sf::Music* music)
  {
    music->stop();
  }

  bool lua_musicIsPlaying(sf::Music* music)
  {
    return music->getStatus() == sf::Music::Playing;
  }

  ////////////////////////////////////////////////////////////////////////////
  // Graphic functions
  ////////////////////////////////////////////////////////////////////////////
  void lua_drawTexture(sf::RenderTarget* target, sf::Texture* texture, int x, int y)
  {
    sf::Sprite sprite;
    sprite.setTexture(*texture);
    sprite.setPosition(x, y);
    target->draw(sprite);
  }

  void lua_drawMessageBox(sf::RenderTarget* target)
  {
    Message::instance().draw(*target);
  }

  void lua_drawMenu(sf::RenderTarget* target, Menu* menu, int x, int y)
  {
    menu->draw(*target, x, y);
  }

  void lua_drawFrame(sf::RenderTarget* target, int x, int y, int w, int h)
  {
    draw_frame(*target, x, y, w, h);
  }

  void lua_drawText(sf::RenderTarget* target, const std::string& text, int x, int y)
  {
    draw_text_bmp(*target, x, y, "%s", text.c_str());
  }

  ////////////////////////////////////////////////////////////////////////////
  // Menu functions
  ////////////////////////////////////////////////////////////////////////////
  ChoiceMenu* lua_createChoiceMenu()
  {
    ChoiceMenu* choiceMenu = new ChoiceMenu;
    return choiceMenu;
  }

  void lua_freeMenu(Menu* menu)
  {
    delete menu;
  }

  void lua_moveMenuArrow(Menu* menu, int dir)
  {
    Direction direction = static_cast<Direction>(dir);
    menu->moveArrow(direction);
  }

  ////////////////////////////////////////////////////////////////////////////
  // Event functions
  ////////////////////////////////////////////////////////////////////////////
  int lua_getEventType(sf::Event* event)
  {
    return static_cast<int>(event->type);
  }
}

void run_lua_script(lua::LuaEnv& luaState, const std::string& script)
{
  if (!luaState.executeFile(config::res_path(script)))
  {
    TRACE("Unable to run script %s: %s", script.c_str(), luaState.getError().c_str());
  }
}

void run_lua_string(lua::LuaEnv& luaState, const std::string& line)
{
  if (!luaState.executeLine(line))
  {
    TRACE("%s", luaState.getError().c_str());
  }
}

void register_lua_bindings(lua::LuaEnv& luaState)
{
  luaState.register_global("GAME_RES_X", config::GAME_RES_X);
  luaState.register_global("GAME_RES_Y", config::GAME_RES_Y);

  luaState.register_global("FADE_NONE", static_cast<int>(Scene::FADE_NONE));
  luaState.register_global("FADE_IN", static_cast<int>(Scene::FADE_IN));
  luaState.register_global("FADE_OUT", static_cast<int>(Scene::FADE_OUT));

  luaState.register_global("DIR_UP", static_cast<int>(DIR_UP));
  luaState.register_global("DIR_DOWN", static_cast<int>(DIR_DOWN));
  luaState.register_global("DIR_LEFT", static_cast<int>(DIR_LEFT));
  luaState.register_global("DIR_RIGHT", static_cast<int>(DIR_RIGHT));

  luaState.register_global("KeyPressed", static_cast<int>(sf::Event::KeyPressed));

  luaState.register_global("Key_Space", static_cast<int>(sf::Keyboard::Space));
  luaState.register_global("Key_Up", static_cast<int>(sf::Keyboard::Up));
  luaState.register_global("Key_Down", static_cast<int>(sf::Keyboard::Down));
  luaState.register_global("Key_Left", static_cast<int>(sf::Keyboard::Left));
  luaState.register_global("Key_Right", static_cast<int>(sf::Keyboard::Right));
  luaState.register_global("Key_Escape", static_cast<int>(sf::Keyboard::Escape));

  lua::reg{luaState}
    // Misc functions
    ("set_global", [](const std::string& globalName, int value) { Persistent<int>::instance().set(globalName, value); })
    ("get_global", [](const std::string& globalName) { return global<int>(globalName); })
    ("enable_encounters", [](bool enabled) { config::ENCOUNTERS_ENABLED = enabled; })
    ("skill_trainer", []() { SceneManager::instance().addScene( new SkillTrainer({"Cartography", "Swimming", "Lockpicking"})); })
    ("start_encounter", [](const std::string& encounterName)
      {
        const Encounter* enc = get_encounter(encounterName);
        if (enc)
        {
          enc->start();
        }
        else
        {
          TRACE("Could not find: %s", encounterName.c_str());
        }
      })
    ("get_config_var", [](const std::string& var) { return config::get(var); })

    // Character functions
    ("afflict_status", &Character::afflictStatus)
    ("cure_status", &Character::cureStatus)
    ("check_vs_luck", check_vs_luck)
    ("get_attribute", &Character::computeCurrentAttribute)
    ("deal_damage", &Character::takeDamage)
    ("get_character_name", &Character::getName)
    ("character_has_status", &Character::hasStatus)
    ("get_current_attribute", [](Character* chr, const std::string& attr) { return chr->getAttribute(attr).current; })
    ("get_max_attribute", [](Character* chr, const std::string& attr) { return chr->getAttribute(attr).max; })

    // Item functions
    ("create_item", [](const std::string& itemName, int amount) { get_player()->addItemToInventory(itemName, amount); })

    // Player functions
    ("get_party_size", []() { return get_player()->getParty().size(); })
    ("get_party_member", [](int index) { return get_player()->getParty().at(index); })
    ("get_player", get_player)
    ("recover_party", &Player::recoverAll)

    // Message functions
    ("message", [](std::string msg) { show_message(msg.c_str()); })
    ("clear_message", []() { Message::instance().clear(); })
    ("update_message", []() { Message::instance().update(); })
    ("message_waiting_for_key", []() { return Message::instance().isWaitingForKey(); })
    ("message_flush", []() { Message::instance().flush(); })

    // Graphics stuff
    ("load_texture", cache::loadTexture)
    ("free_texture", (void (*) (sf::Texture*)) cache::releaseTexture)
    ("draw_texture", lua_drawTexture)
    ("draw_message_box", lua_drawMessageBox)
    ("draw_menu", lua_drawMenu)
    ("draw_frame", lua_drawFrame)
    ("draw_text", lua_drawText)

    // Audio stuff
    ("play_sound", [](std::string sound) { play_sound(sound); })
    ("load_music", lua_loadMusic)
    ("free_music", lua_freeMusic)
    ("play_music", lua_playMusic)
    ("stop_music", lua_stopMusic)
    ("music_is_playing", lua_musicIsPlaying)

    // Scene functions
    ("shake_screen", [](int duration, int power) { SceneManager::instance().shakeScreen(duration, power, power); })
    ("fade_in", [](int duration) { SceneManager::instance().fadeIn(duration); })
    ("fade_out", [](int duration) { SceneManager::instance().fadeOut(duration); })
    ("close", Scene::close)

    // Menu functions
    ("create_choice_menu", lua_createChoiceMenu)
    ("free_menu", lua_freeMenu)
    ("menu_set_visible", &Menu::setVisible)
    ("menu_add_entry", &Menu::addEntry)
    ("menu_is_visible", &Menu::isVisible)
    ("get_menu_width", &Menu::getWidth)
    ("get_menu_height", &Menu::getHeight)
    ("get_current_menu_choice", &Menu::getCurrentMenuChoice)
    ("move_menu_arrow", lua_moveMenuArrow)

    // Event functions
    ("event_type", lua_getEventType);
}
