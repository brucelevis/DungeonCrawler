#ifndef GUIWIDGET_H_
#define GUIWIDGET_H_

#include <SFML/Graphics.hpp>
#include "Direction.h"

class GuiStack;

class GuiWidget
{
public:
  GuiWidget();
  virtual ~GuiWidget();

  void setVisible(bool visible) { m_visible = visible; }
  bool isVisible() const { return m_visible; }

  void setCursorVisible(bool visible) { m_cursorVisible = visible; }
  bool cursorVisible() const { return m_cursorVisible; }

  virtual void update() {}
  virtual bool handleInput(sf::Keyboard::Key key) = 0;
  virtual void draw(sf::RenderTarget& target) = 0;
protected:
  GuiStack* getGuiStack() const;
  void drawSelectArrow(sf::RenderTarget& target, int x, int y);
  void drawTopScrollArrow(sf::RenderTarget& target, int x, int y);
  void drawBottomScrollArrow(sf::RenderTarget& target, int x, int y);
private:
  friend class GuiStack;
  void setGuiStack(GuiStack* guiStack);
private:
  sf::Texture* m_arrowTexture;

  bool m_visible;
  bool m_cursorVisible;

  GuiStack* m_guiStack;
};

#endif /* GUIWIDGET_H_ */
