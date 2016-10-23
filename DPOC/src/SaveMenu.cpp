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
 : m_type(type)
{
  refresh();

  m_range = Range(0, NUM_SAVE_FILES, NUM_VISIBLE_SAVES);
}

bool SaveMenu::handleInput(sf::Keyboard::Key key)
{
  switch (key)
  {
  case sf::Keyboard::Up:
    m_range.subIndex(1, Range::WRAP);
    break;
  case sf::Keyboard::Down:
    m_range.addIndex(1, Range::WRAP);
    break;
  case sf::Keyboard::Space:
  case sf::Keyboard::Return:
    handleConfirm();
    break;
  case sf::Keyboard::Escape:
    getGuiStack()->removeWidget(this);
    break;
  default:
    break;
  }

  return true;
}

void SaveMenu::draw(sf::RenderTarget& target)
{
  const int width = 4 + get_longest_string(m_slotNames).size() * 8;
  const int height = 2 * 8 + NUM_VISIBLE_SAVES * ENTRY_OFFSET;
  const int x = target.getSize().x / 2 - width / 2;
  const int y = target.getSize().y / 2 - height / 2;

  draw_frame(target, x, y, width, height);

  for (int index = m_range.getStart(), i = 0; index <= m_range.getEnd(); index++, i++)
  {
    if (index < (int)m_slotNames.size())
    {
      draw_text_bmp(target, x + 16, y + 8 + i * ENTRY_OFFSET, "%s", m_slotNames[index].c_str());
    }

    if (m_range.getIndex() == index && cursorVisible())
    {
      drawSelectArrow(target, x + 8, y + 8 + i * ENTRY_OFFSET);
    }
  }

  if (m_range.getStart() > m_range.getMin())
  {
    drawTopScrollArrow(target, x + width - 12, y + 4);
  }

  if (m_range.getEnd() < m_range.getMax())
  {
    drawBottomScrollArrow(target, x + width - 12, y + height - 12);
  }
}

void SaveMenu::handleConfirm()
{
  if (m_type == SAVE)
  {
    save_game(m_filenames[m_range.getIndex()]);
    play_sound(config::get("SOUND_SUCCESS"));

    refresh();
  }
  else
  {
    if (file_exists(config::res_path("Saves/" + m_filenames[m_range.getIndex()])))
    {
      load_game(m_filenames[m_range.getIndex()]);

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
  m_slotNames.clear();

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

    m_slotNames.push_back(slotName);
  }
}
