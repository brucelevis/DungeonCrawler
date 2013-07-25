#ifndef SAVE_MENU_H
#define SAVE_MENU_H

#include <vector>
#include <string>

#include "Menu.h"

class SaveMenu : public Menu
{
public:
  enum SaveOrLoad
  {
    SAVE,
    LOAD
  };

  SaveMenu(SaveOrLoad type);

  void handleConfirm();
private:
  void refresh();
private:
  SaveOrLoad m_type;
  std::vector<std::string> m_filenames;
};

#endif
