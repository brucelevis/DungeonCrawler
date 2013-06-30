#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <queue>

#include <SFML/Graphics.hpp>

class Message
{
public:
  static Message& instance();
  void show(const std::string& msg, bool append = false);
  void nextPage();
  void flush();

  void clear();

  bool isVisible() const
  {
    return !m_pages.empty();
  }
  bool isWaitingForKey() const
  {
    if (m_pages.empty())
      return false;

    return m_currentIndex == m_pages.front().size();
  }

  void update();
  void draw(sf::RenderTarget& target);
private:
  Message() : m_currentIndex(0) {}
private:
  std::queue<std::string> m_pages;
  std::string m_currentBuffer;
  size_t m_currentIndex;
};

// Thin wrappers around singleton.
void show_message(const char* fmt, ...);
void update_message();
void battle_message(const char* fmt, ...);

#endif
