#include "draw_text.h"
#include "Console.h"

Console::Console()
 : m_isOpen(false)
{

}

void Console::add(const std::string& str)
{
  if (m_buffer.size() > 100)
  {
    m_buffer.pop_front();
  }

  m_buffer.push_back(str);
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
  rect.setFillColor(sf::Color(0, 127, 255, 200));
  target.draw(rect);

  for (size_t i = 0; i < m_buffer.size(); i++)
  {
    draw_text(target, 4, 4 + i * 14, "%s", m_buffer[i].c_str());
  }
}
