#include <algorithm>
#include "GuiStack.h"

void GuiStack::update()
{
  for (auto& widget : m_guiWidgets)
  {
    widget->update();
  }
}

bool GuiStack::handleEvent(const sf::Event& event)
{
  if (event.type == sf::Event::KeyPressed)
  {
    sf::Keyboard::Key key = event.key.code;

    for (auto rit = m_guiWidgets.rbegin(); rit != m_guiWidgets.rend(); ++rit)
    {
      GuiWidget* widget = rit->get();

      if (widget->handleInput(key))
      {
        return true;
      }
    }
  }

  return false;
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

void GuiStack::removeWidget(const GuiWidget* widget)
{
  auto it = std::remove_if(m_guiWidgets.begin(), m_guiWidgets.end(), [&widget](const std::unique_ptr<GuiWidget>& w)
  {
    return w.get() == widget;
  });

  m_guiWidgets.erase(it, m_guiWidgets.end());
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
