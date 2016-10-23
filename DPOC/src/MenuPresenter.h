#ifndef MENUPRESENTER_H_
#define MENUPRESENTER_H_

#include <vector>
#include <string>

#include <SFML/Graphics.hpp>

#include "Range.h"

class GuiWidget;

class MenuPresenter
{
public:
  struct Entry
  {
    std::string entryName;
    int entryIndex;
  };

  enum Style
  {
    NO_STYLE,
    STYLE_FRAME
  };

  MenuPresenter(Style style = NO_STYLE);

  void clear();
  void reset();

  void scrollUp();
  void scrollDown();

  void setMaxVisible(int maxVisible);
  void addEntry(const std::string& entryName);

  int getWidth() const;
  int getHeight() const;

  Entry getSelectedOption() const;

  void draw(sf::RenderTarget& target, int x, int y, const GuiWidget* guiWidget) const;
private:
  std::vector<std::string> m_options;
  Range m_range;
  Style m_style;
};

#endif /* MENUPRESENTER_H_ */
