#ifndef SHOP_H
#define SHOP_H

#include <vector>
#include <string>

#include <SFML/Graphics.hpp>

#include "Scene.h"
#include "Direction.h"
#include "Menu.h"

class PlayerCharacter;
class ConfirmMenu;

class ShopBuyMenu : public Menu
{
public:
  ShopBuyMenu(const std::vector<std::string>& inventory);
  ~ShopBuyMenu();

  void handleConfirm();
  void handleEscape();
  void moveArrow(Direction dir);

  int getWidth() const;
  int getHeight() const;

  void draw(sf::RenderTarget& target, int x, int y);
private:
  void drawDeltas(sf::RenderTarget& target, PlayerCharacter* character, const std::string& itemName, int x, int y);
private:
  std::vector<std::string> m_inventory;
  ConfirmMenu* m_confirmMenu;
};

class ShopSellMenu : public Menu
{
public:
  void handleConfirm() {}
private:
};

class ShopMenu : public Menu
{
public:
  ShopMenu();
  ~ShopMenu();

  void moveArrow(Direction dir);
  void handleConfirm();
  void handleEscape();

  void draw(sf::RenderTarget& target, int x, int y);
private:
  ShopBuyMenu* m_buyMenu;
  ShopSellMenu* m_sellMenu;

  std::vector<std::string> m_inventory;
};

class Shop : public Scene
{
public:
  void update();
  void draw(sf::RenderTarget& target);
  void handleEvent(sf::Event& event);
private:
  void handleKeyPress(sf::Keyboard::Key key);
private:
  ShopMenu m_menu;
};

#endif
