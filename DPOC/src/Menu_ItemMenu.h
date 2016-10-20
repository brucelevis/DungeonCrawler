#ifndef MENU_ITEMMENU_H_
#define MENU_ITEMMENU_H_

#include <vector>
#include <string>
#include <functional>

#include "Item.h"
#include "Range.h"
#include "GuiWidget.h"

class ItemMenu : public GuiWidget
{
public:
  using Callback = std::function<void(const Item*)>;

  ItemMenu(const Callback& callback, int x, int y, int w = 14*16, int h = 12*16);

  bool handleInput(sf::Keyboard::Key key) override;
  void draw(sf::RenderTarget& target) override;

  void refresh();

  std::string getSelectedItemName() const;
private:
  bool hasItem(const std::string& name) const;
  const Item* getItem(const std::string& name) const;
protected:
  std::vector<const Item*> m_items;

  int m_x, m_y;
  int m_width, m_height;

  Range m_itemRange;

  Callback m_callback;
};

#endif /* MENU_ITEMMENU_H_ */
