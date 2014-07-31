#include <ctime>

#include "LuaBindings.h"
#include "logger.h"
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

  bool _valid(char c)
  {
    return c >= 32;
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
    draw_text_bmp(target, 4, 4 + i * 8, "%s", m_buffer[i].c_str());
  }

  // Draw prompt
  rect.setSize(sf::Vector2f(target.getSize().x, 14));
  rect.setPosition(0, target.getSize().y / 2);
  rect.setFillColor(sf::Color(0, 0, 0, 230));
  target.draw(rect);

  draw_text_bmp_ex(target, 4, target.getSize().y / 2 + 4, sf::Color::Yellow, ">>%s_", m_currentInput.c_str());
}

void Console::addInput(char c)
{
  if (c == '\n' || c == '\r')
  {
    TRACE("%s", m_currentInput.c_str());

    run_lua_string(m_currentInput);

    m_currentInput.clear();
  }
  else if (c == '\b')
  {
    if (m_currentInput.size())
    {
      m_currentInput.pop_back();
    }
  }
  else if (c == '\x1B')
  {
    m_currentInput.clear();
  }
  else if (_valid(c))
  {
    m_currentInput += c;
  }
}
