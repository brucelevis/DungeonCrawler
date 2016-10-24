#include <string>
#include <cstdio>

#include "Frame.h"
#include "Sound.h"
#include "Config.h"
#include "SaveLoad.h"
#include "Utility.h"
#include "GuiStack.h"
#include "draw_text.h"

#include "SaveMenu.h"

const int NUM_VISIBLE_SAVES = 8;
const int NUM_SAVE_FILES = 16;

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
 : m_type(type),
   m_presenter(MenuPresenter::STYLE_FRAME)
{
  refresh();
}

bool SaveMenu::handleInput(sf::Keyboard::Key key)
{
  switch (key)
  {
  case sf::Keyboard::Up:
    m_presenter.scrollUp();
    break;
  case sf::Keyboard::Down:
    m_presenter.scrollDown();
    break;
  case sf::Keyboard::Space:
  case sf::Keyboard::Return:
    handleConfirm();
    break;
  case sf::Keyboard::Escape:
    close();
    break;
  default:
    break;
  }

  return true;
}

void SaveMenu::draw(sf::RenderTarget& target)
{
  const int x = target.getSize().x / 2 - m_presenter.getWidth() / 2;
  const int y = target.getSize().y / 2 - m_presenter.getHeight() / 2;

  m_presenter.draw(target, x, y, this);
}

void SaveMenu::handleConfirm()
{
  auto entry = m_presenter.getSelectedOption();

  if (m_type == SAVE)
  {
    save_game(m_filenames[entry.entryIndex]);
    play_sound(config::get("SOUND_SUCCESS"));

    refresh();
  }
  else
  {
    if (file_exists(config::res_path("Saves/" + m_filenames[entry.entryIndex])))
    {
      load_game(m_filenames[entry.entryIndex]);

      if (m_callback)
      {
        m_callback();
      }
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
  m_presenter.clear();

  for (int i = 0; i < NUM_SAVE_FILES; i++)
  {
    std::string filename = "Save" + toString(i) + ".xml";

    m_filenames.push_back(filename);

    std::string slotName = "Save Slot " + toString(i+1);

    if (file_exists(path + filename))
    {
      CharacterData leader = get_party_leader_from_save(filename);

      slotName += " {" + limit_string(leader.name, 3) + " L" + toString(leader.attributes["level"].max) + "}";
    }

    m_presenter.addEntry(slotName);
  }

  m_presenter.setMaxVisible(NUM_VISIBLE_SAVES);
}
