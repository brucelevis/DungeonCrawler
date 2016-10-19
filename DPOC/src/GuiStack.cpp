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

void GuiStack::removeWidget(const GuiWidget* widget)
{
  auto it = std::remove_if(m_guiWidgets.begin(), m_guiWidgets.end(), [&widget](const std::unique_ptr<GuiWidget>& w)
  {
    return w.get() == widget;
  });

  m_guiWidgets.erase(it, m_guiWidgets.end());
}
