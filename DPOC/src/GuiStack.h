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

  template <typename T, typename... Args>
  T* addWidget(Args&&... args)
  {
    std::unique_ptr<T> ptr{new T{std::forward<Args>(args)...}};
    ptr->setGuiStack(this);
    T* rawPtr = static_cast<T*>(ptr.get());
    m_guiWidgets.emplace_back(std::move(ptr));
    m_guiWidgets.back()->start();
    return rawPtr;
  }

  void bringToFront(const GuiWidget* widget);
  void yield(const GuiWidget* widget);

  template <typename T>
  T* findWidget()
  {
    for (auto& w : m_guiWidgets)
    {
      if (dynamic_cast<T*>(w.get()))
      {
        return dynamic_cast<T*>(w.get());
      }
    }

    return nullptr;
  }

  GuiWidget* getTop();

  void draw(sf::RenderTarget& target);
private:
  std::vector<std::unique_ptr<GuiWidget>>::iterator findIterator(const GuiWidget* widget);
  void cleanup();
private:
  std::vector<std::unique_ptr<GuiWidget>> m_guiWidgets;
};

#endif /* GUISTACK_H_ */
