#ifndef SAVE_MENU_H
#define SAVE_MENU_H

#include <vector>
#include <string>

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

  SaveMenu(SaveOrLoad type);

  bool handleInput(sf::Keyboard::Key key) override;
  void draw(sf::RenderTarget& target) override;
private:
  void handleConfirm();
  void refresh();
private:
  SaveOrLoad m_type;
  std::vector<std::string> m_filenames;
  std::vector<std::string> m_slotNames;
  Range m_range;
};

#endif
