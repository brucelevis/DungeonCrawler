#include <functional>

#include "SceneManager.h"
#include "Menu_ShopMenu.h"
#include "Shop.h"

Shop::Shop(const std::vector<std::string>& items)
{
  SceneManager::instance().getGuiStack()->addWidget<ShopMenu>(items, std::bind(&Shop::menuClosed, this));
}

void Shop::update()
{
}

void Shop::draw(sf::RenderTarget& target)
{
}

void Shop::handleEvent(sf::Event& event)
{
  switch (event.type)
  {
  case sf::Event::KeyPressed:
    handleKeyPress(event.key.code);
    break;
  default:
    break;
  }
}

void Shop::handleKeyPress(sf::Keyboard::Key key)
{
}

void Shop::menuClosed()
{
  close();
}
