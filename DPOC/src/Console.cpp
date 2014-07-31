#include <ctime>

#include "draw_text.h"
#include "Console.h"

namespace
{
  std::string _get_timestamp()
  {
    char buffer[64];

    time_t the_time;
    tm* timeinfo;

    time(&the_time);
    timeinfo = localtime(&the_time);

    strftime(buffer, 64, "%H:%M:%S", timeinfo);

    return buffer;
  }
}

const size_t Console::MAX_LOG_SIZE = 28;

Console::Console()
 : m_isOpen(false)
{

}

void Console::add(const std::string& str)
{
  std::string timestamp = _get_timestamp();

  if (m_buffer.size() > MAX_LOG_SIZE)
  {
    m_buffer.pop_front();
  }

  m_buffer.push_back("[" + timestamp + "] " + str);
}

void Console::setOpen(bool isOpen)
{
  m_isOpen = isOpen;
}

bool Console::isOpen() const
{
  return m_isOpen;
}

void Console::draw(sf::RenderTarget& target) const
{
  sf::RectangleShape rect;
  rect.setSize(sf::Vector2f(target.getSize().x, target.getSize().y / 2));
  rect.setFillColor(sf::Color(0, 127, 255, 230));
  target.draw(rect);

  for (size_t i = 0; i < m_buffer.size(); i++)
  {
    //draw_text(target, 4, 4 + i * 14, "%s", m_buffer[i].c_str());
    draw_text_bmp(target, 4, 4 + i * 8, "%s", m_buffer[i].c_str());
  }
}
