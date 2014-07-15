#include <string>
#include <cstdio>

#include "Sound.h"
#include "Config.h"
#include "SaveLoad.h"
#include "Utility.h"
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
    play_sound(config::get("SOUND_SUCCESS"));

    refresh();
  }
  else
  {
    if (file_exists(config::res_path("Saves/" + m_filenames[getCurrentChoiceIndex()])))
    {
      load_game(m_filenames[getCurrentChoiceIndex()]);

      setVisible(false);
    }
    else
    {
      play_sound(config::get("SOUND_CANCEL"));
    }
  }
}

void SaveMenu::refresh()
{
  std::string path = config::res_path("Saves/");

  m_filenames.clear();
  clear();

  for (int i = 0; i < 16; i++)
  {
    std::string filename = "Save" + toString(i) + ".xml";

    m_filenames.push_back(filename);

    std::string slotName = "Save Slot " + toString(i+1);

    if (file_exists(path + filename))
    {
      CharacterData leader = get_party_leader_from_save(filename);

      slotName += " {" + limit_string(leader.name, 3) + " L" + toString(leader.attributes["level"].max) + "}";
    }

    addEntry(slotName);
  }
}
