#include <fstream>
#include <sstream>

#include "logger.h"

#include "Game.h"
#include "Entity.h"
#include "Map.h"
#include "Player.h"
#include "PlayerCharacter.h"
#include "Character.h"
#include "Persistent.h"

#include "SaveLoad.h"

void save_game(const std::string& saveFile)
{
  TRACE("Saving game to: %s", saveFile.c_str());

  std::ostringstream xml;
  xml << "<save>\n";

  xml << Game::instance().getCurrentMap()->xmlDump();
  xml << get_player()->xmlDump();
  xml << Persistent<int>::instance().xmlDump();

  xml << "</save>\n";

  TRACE("XmlSave:\n%s", xml.str().c_str());

  std::ofstream ofile("Resources/Saves/" + saveFile);
  ofile << xml.str();
  ofile.close();
}

void load_game(const std::string& saveFile)
{
  TRACE("Loading game: %s", saveFile.c_str());
}
