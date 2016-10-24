#include <algorithm>
#include "GuiStack.h"

void GuiStack::update()
{
  for (auto& widget : m_guiWidgets)
  {
    widget->update();
  }

  cleanup();
}

bool GuiStack::handleEvent(const sf::Event& event)
{
  bool eventHandled = false;

  if (event.type == sf::Event::KeyPressed)
  {
    sf::Keyboard::Key key = event.key.code;

    for (auto rit = m_guiWidgets.rbegin(); rit != m_guiWidgets.rend(); ++rit)
    {
      GuiWidget* widget = rit->get();

      if (widget->handleInput(key))
      {
        eventHandled = true;
        break;
      }
    }

    cleanup();
  }

  return eventHandled;
}

void GuiStack::bringToFront(const GuiWidget* widget)
{
  auto it = findIterator(widget);

  if (it == m_guiWidgets.end())
  {
    return;
  }

  std::rotate(it, it + 1, m_guiWidgets.end());
}

void GuiStack::yield(const GuiWidget* widget)
{
  if (m_guiWidgets.size() > 1 && m_guiWidgets.front().get() != widget)
  {
    auto it = findIterator(widget);

    std::iter_swap(it, it-1);
  }
}

GuiWidget* GuiStack::getTop()
{
  if (m_guiWidgets.size())
  {
    return m_guiWidgets.back().get();
  }

  return nullptr;
}

void GuiStack::draw(sf::RenderTarget& target)
{
  for (auto& widget : m_guiWidgets)
  {
    if (widget->isVisible())
    {
      widget->draw(target);
    }
  }
}

std::vector<std::unique_ptr<GuiWidget>>::iterator GuiStack::findIterator(const GuiWidget* widget)
{
  auto it = std::find_if(m_guiWidgets.begin(), m_guiWidgets.end(), [&widget](const std::unique_ptr<GuiWidget>& w)
  {
    return w.get() == widget;
  });

  return it;
}

void GuiStack::cleanup()
{
  // Remove if widget is in toBeRemoved list.
  auto it = std::remove_if(m_guiWidgets.begin(), m_guiWidgets.end(), [this](const std::unique_ptr<GuiWidget>& w)
  {
    return w->isOpen() == false;
  });

  m_guiWidgets.erase(it, m_guiWidgets.end());

  // Continue cleanup if more widgets has been closed.
  if (m_guiWidgets.end() != std::find_if(m_guiWidgets.begin(), m_guiWidgets.end(), [&](const std::unique_ptr<GuiWidget>& w) { return w->isOpen() == false; }))
  {
    cleanup();
  }
}
