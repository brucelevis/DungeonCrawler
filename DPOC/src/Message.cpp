#include <sstream>
#include <vector>
#include <cstdarg>

#include "Sound.h"
#include "Config.h"
#include "draw_text.h"
#include "Utility.h"
#include "Frame.h"
#include "Message.h"

static const size_t CHARS_PER_LINE = 30;

static std::vector<std::string> battleMessage;

void show_message(const char* fmt, ...)
{
  char buffer[512];

  va_list args;
  va_start(args, fmt);

  vsprintf(buffer, fmt, args);

  va_end(args);

  Message::instance().show(buffer);
}

void update_message()
{
  if (Message::instance().isVisible())
  {
    Message::instance().update();
  }
}

void clear_message()
{
  Message::instance().clear();
  battleMessage.clear();
}

void battle_message(const char* fmt, ...)
{
  char buffer[512];

  va_list args;
  va_start(args, fmt);

  vsprintf(buffer, fmt, args);

  va_end(args);

  std::vector<std::string> strings = split_string(buffer, ' ');

  static const size_t WORDS = 256 / 8 - 2;

  std::string tmp = strings[0];
  for (size_t i = 1; i < strings.size(); i++)
  {
    std::string s = tmp + " " + strings[i];
    if (s.size() >= WORDS)
    {
      battleMessage.push_back(tmp);
      tmp = strings[i];
    }
    else
    {
      tmp = s;
    }
  }

  if (tmp.size() > 0)
  {
    battleMessage.push_back(tmp);
  }
}

void draw_battle_message(sf::RenderTarget& target)
{
  for (size_t i = 0; i < battleMessage.size(); i++)
  {
    draw_text_bmp(target, 8, 8 + i * 12, "%s", battleMessage[i].c_str());
  }
}

Message& Message::instance()
{
  static Message message;
  return message;
}

void Message::show(const std::string& msg, bool append)
{
  std::vector<std::string> strings;

  if (m_pages.empty() && append)
    append = false;

  if (!append)
  {
    strings = split_string(msg, ' ');
  }
  else
  {
    strings = split_string(m_pages.front() + "\n" + msg, ' ');
    m_pages.front().clear();
  }

  std::string buffer;
  std::string complete;
  int lines = 0;
  for (auto it = strings.begin(); it != strings.end();)
  {
    std::string tmp = buffer + (*it);

    if (tmp.size() > CHARS_PER_LINE)
    {
      if (buffer[buffer.size() - 1] == ' ')
      {
        buffer.resize(buffer.size() - 1);
      }

      complete += buffer + "\n";
      buffer.clear();

      lines++;
      if (lines >= 4)
      {
        lines = 0;
        if (!append) m_pages.push(complete);
        else m_pages.front() = complete;
        complete.clear();
      }
    }
    else
    {
      buffer += (*it) + " ";
      ++it;
    }
  }

  if (!buffer.empty())
    complete += buffer;

  if (!complete.empty())
  {
    if (!append) m_pages.push(complete);
    else m_pages.front() = complete;
  }

}

void Message::nextPage()
{
  m_pages.pop();
  m_currentBuffer.clear();
  m_currentIndex = 0;
}

void Message::flush()
{
  if (m_pages.empty())
    return;

  bool quiet = m_quiet;

  if (!quiet)
    setIsQuiet(true);

  while (m_currentIndex < m_pages.front().size())
  {
    update();
  }

  if (!quiet)
    setIsQuiet(false);
}

void Message::clear()
{
  while (!m_pages.empty())
    nextPage();
}

void Message::update()
{
  if (!m_pages.empty() && m_currentIndex < m_pages.front().size())
  {
    m_currentBuffer += m_pages.front()[m_currentIndex];

    if (!m_quiet && !config::get("SOUND_MESSAGE_INCREMENT").empty())
    {
      play_sound(config::get("SOUND_MESSAGE_INCREMENT"));
    }

    m_currentIndex++;
  }
}

void Message::draw(sf::RenderTarget& target)
{
  draw_frame(target, config::GAME_RES_X / 2 - 256 / 2, config::GAME_RES_Y - 48, 256, 48);
  draw_text_bmp(target, 8 + config::GAME_RES_X / 2 - 256 / 2, config::GAME_RES_Y - 48 + 8, "%s", m_currentBuffer.c_str());
}
