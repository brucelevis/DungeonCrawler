#ifndef SAVE_MENU_H
#define SAVE_MENU_H

#include <vector>
#include <string>
#include <functional>

#include "Range.h"
#include "GuiWidget.h"

class SaveMenu : public GuiWidget
{
public:
  enum SaveOrLoad
  {
    SAVE,
    LOAD
  };

  using LoadGameCallback = std::function<void()>;

  SaveMenu(SaveOrLoad type);

  bool handleInput(sf::Keyboard::Key key) override;
  void draw(sf::RenderTarget& target) override;

  void setLoadGameCallback(const LoadGameCallback& callback)
  {
    m_callback = callback;
  }
private:
  void handleConfirm();
  void refresh();
private:
  SaveOrLoad m_type;
  std::vector<std::string> m_filenames;
  std::vector<std::string> m_slotNames;
  Range m_range;
  LoadGameCallback m_callback;
};

#endif
