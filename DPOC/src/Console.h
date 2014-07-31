#ifndef CONSOLE_H
#define CONSOLE_H

#include <deque>
#include <string>

#include <SFML/Graphics.hpp>

class Console
{
  static const size_t MAX_LOG_SIZE = 100;
public:
  Console();

  void add(const std::string& str);

  void setOpen(bool isOpen);
  bool isOpen() const;
  void draw(sf::RenderTarget& target) const;
private:
  bool m_isOpen;
  std::deque<std::string> m_buffer;
};

#endif
