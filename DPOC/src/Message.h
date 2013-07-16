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

  bool lastMessage() const
  {
	return isWaitingForKey() && m_pages.size() == 1;
  }

  std::string currentMessage() const
  {
    if (m_pages.size() > 0)
      return m_pages.front();
    return "";
  }

  void update();
  void draw(sf::RenderTarget& target);

  void setIsQuiet(bool b) { m_quiet = b; }
private:
  Message() : m_currentIndex(0), m_quiet(false) {}
private:
  std::queue<std::string> m_pages;
  std::string m_currentBuffer;
  size_t m_currentIndex;

  bool m_quiet;
};

// Thin wrappers around singleton.
void show_message(const char* fmt, ...);
void update_message();
void clear_message();
void battle_message(const char* fmt, ...);
void draw_battle_message(sf::RenderTarget& target);

#endif
