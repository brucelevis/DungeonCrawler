#include "Cache.h"
#include "GuiWidget.h"

GuiWidget::GuiWidget()
  : m_arrowTexture(cache::loadTexture("UI/Arrow.png")),
    m_visible(true),
    m_cursorVisible(true),
    m_guiStack(nullptr)
{
}

GuiWidget::~GuiWidget()
{
  cache::releaseTexture(m_arrowTexture);
}

GuiStack* GuiWidget::getGuiStack() const
{
  return m_guiStack;
}

void GuiWidget::setGuiStack(GuiStack* guiStack)
{
  m_guiStack = guiStack;
}

void GuiWidget::drawSelectArrow(sf::RenderTarget& target, int x, int y) const
{
  sf::Sprite sprite;
  sprite.setTexture(*m_arrowTexture);
  sprite.setTextureRect(sf::IntRect(0, 0, 8, 8));
  sprite.setPosition(x, y);
  target.draw(sprite);
}

void GuiWidget::drawTopScrollArrow(sf::RenderTarget& target, int x, int y) const
{
  sf::Sprite sprite;
  sprite.setTexture(*m_arrowTexture);
  sprite.setPosition(x, y);
  sprite.setTextureRect(sf::IntRect(8, 0, 8, 8));
  target.draw(sprite);
}

void GuiWidget::drawBottomScrollArrow(sf::RenderTarget& target, int x, int y) const
{
  sf::Sprite sprite;
  sprite.setTexture(*m_arrowTexture);
  sprite.setPosition(x, y);
  sprite.setTextureRect(sf::IntRect(16, 0, 8, 8));
  target.draw(sprite);
}
