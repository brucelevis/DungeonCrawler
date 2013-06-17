#include <sstream>
#include <vector>

#include "Config.h"
#include "draw_text.h"
#include "Utility.h"
#include "Frame.h"
#include "Message.h"

static const size_t CHARS_PER_LINE = 30;

Message& Message::instance()
{
  static Message message;
  return message;
}

void Message::show(const std::string& msg)
{
  std::vector<std::string> strings = split_string(msg, ' ');

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
        m_pages.push(complete);
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
    m_pages.push(complete);
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

  while (m_currentIndex < m_pages.front().size())
  {
    update();
  }
}

void Message::update()
{
  if (!m_pages.empty() && m_currentIndex < m_pages.front().size())
  {
    m_currentBuffer += m_pages.front()[m_currentIndex];

    m_currentIndex++;
  }
}

void Message::draw(sf::RenderTarget& target)
{
  draw_frame(target, 0, config::GAME_RES_Y - 48, 256, 48);
  draw_text_bmp(target, 8, config::GAME_RES_Y - 48 + 8, "%s", m_currentBuffer.c_str());
}
