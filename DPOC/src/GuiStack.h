#ifndef GUISTACK_H_
#define GUISTACK_H_

#include <memory>
#include <vector>

#include "GuiWidget.h"

class GuiStack
{
public:
  void update();

  bool handleEvent(const sf::Event& event);

  template <typename T, typename ... Args>
  T* addWidget(Args&& ... args)
  {
    std::unique_ptr<T> ptr{new T{std::forward<Args>(args)...}};
    ptr->setGuiStack(this);
    m_guiWidgets.emplace_back(std::move(ptr));
    return m_guiWidgets.back().get();
  }

  void removeWidget(const GuiWidget* widget);

  template <typename T>
  T* findWidget()
  {
    for (auto& w : m_guiWidgets)
    {
      if (dynamic_cast<T*>(w.get()))
      {
        return w.get();
      }
    }

    return nullptr;
  }
private:
  std::vector<std::unique_ptr<GuiWidget>> m_guiWidgets;
};

#endif /* GUISTACK_H_ */
