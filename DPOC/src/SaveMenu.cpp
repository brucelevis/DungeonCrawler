#include <string>
#include <cstdio>

#include <BGL/Sound.h>
#include <BGL/Strings.h>

#include "Config.h"
#include "SaveLoad.h"
#include "SaveMenu.h"

static bool file_exists(const std::string& fileName)
{
  if (FILE* f = fopen(fileName.c_str(), "r"))
  {
    fclose(f);
    return true;
  }

  return false;
}

SaveMenu::SaveMenu(SaveOrLoad type)
 : m_type(type)
{
  refresh();

  setMaxVisible(8);
}

void SaveMenu::handleConfirm()
{
  if (m_type == SAVE)
  {
    save_game(m_filenames[getCurrentChoiceIndex()]);
    bgl::play_sound(config::get("SOUND_SUCCESS"));

    refresh();
  }
  else
  {
    if (file_exists("Resources/Saves/" + m_filenames[getCurrentChoiceIndex()]))
    {
      load_game(m_filenames[getCurrentChoiceIndex()]);

      setVisible(false);
    }
    else
    {
      bgl::play_sound(config::get("SOUND_CANCEL"));
    }
  }
}

void SaveMenu::refresh()
{
  std::string path = "Resources/Saves/";

  m_filenames.clear();
  clear();

  for (int i = 0; i < 16; i++)
  {
    std::string filename = "Save" + bgl::str::toString(i) + ".xml";

    m_filenames.push_back(filename);

    std::string slotName = "Save Slot " + bgl::str::toString(i+1);

    if (file_exists(path + filename))
    {
      CharacterData leader = get_party_leader_from_save(filename);

      slotName += " {" + bgl::str::limit_string(leader.name, 3) + " L" + bgl::str::toString(leader.attributes["level"].max) + "}";
    }

    addEntry(slotName);
  }
}
