#include "Frame.h"
#include "draw_text.h"
#include "Config.h"
#include "Utility.h"
#include "Sound.h"

#include "PlayerCharacter.h"
#include "Player.h"
#include "Entity.h"
#include "Sprite.h"
#include "Direction.h"

#include "Item.h"

#include "Shop.h"

class ConfirmMenu : public Menu
{
public:
  enum BuyOrSell
  {
    BUY, SELL
  };

  ConfirmMenu(BuyOrSell type, const std::string& itemName);

  void moveArrow(Direction dir);
  void handleConfirm();

  void draw(sf::RenderTarget& target, int x, int y);

  int getWidth() const
  {
    return 104;
  }
private:
  int getSum() const;
private:
  int m_quantity;
  BuyOrSell m_type;
  std::string m_itemName;
};

ConfirmMenu::ConfirmMenu(BuyOrSell type, const std::string& itemName)
 : m_quantity(1),
   m_type(type),
   m_itemName(itemName)
{
  addEntry(type == BUY ? "Buy" : "Sell");
  addEntry("Cancel");
}

void ConfirmMenu::moveArrow(Direction dir)
{
  if (dir == DIR_LEFT)
  {
    m_quantity--;
    if (m_quantity < 1)
    {
      m_quantity = 1;
    }
  }
  else if (dir == DIR_RIGHT)
  {
    m_quantity++;

    if (m_type == SELL && m_quantity > get_player()->getItem(m_itemName)->stackSize)
    {
      m_quantity = get_player()->getItem(m_itemName)->stackSize;
    }
    else if (m_quantity > 99)
    {
      m_quantity = 99;
    }
  }
  else
  {
    Menu::moveArrow(dir);
  }
}

void ConfirmMenu::handleConfirm()
{
  std::string choice = getCurrentMenuChoice();

  if (choice == "Buy")
  {
    int stackAfterBuy = m_quantity;
    if (get_player()->getItem(m_itemName))
    {
      stackAfterBuy += get_player()->getItem(m_itemName)->stackSize;
    }

    if (get_player()->getGold() >= getSum() && stackAfterBuy <= 99)
    {
      get_player()->gainGold(-getSum());
      get_player()->addItemToInventory(m_itemName, m_quantity);

      play_sound(config::get("SOUND_SHOP"));

      setVisible(false);
    }
    else
    {
      play_sound(config::get("SOUND_CANCEL"));
    }
  }
  else if (choice == "Sell")
  {
    get_player()->gainGold(getSum());
    get_player()->removeItemFromInventory(m_itemName, m_quantity);

    play_sound(config::get("SOUND_SHOP"));

    setVisible(false);
  }
  else if (choice == "Cancel")
  {
    setVisible(false);
  }
}

void ConfirmMenu::draw(sf::RenderTarget& target, int x, int y)
{
  int owned = 0;
  if (get_player()->getItem(m_itemName))
  {
    owned = get_player()->getItem(m_itemName)->stackSize;
  }

  draw_frame(target, x, y, getWidth(), 40);
  draw_text_bmp(target, x + 8, y + 8,  "Quantity %d", m_quantity);
  draw_text_bmp(target, x + 8, y + 16, "Owned    %d", owned);
  draw_text_bmp(target, x + 8, y + 24, "Sum %d", getSum());

  Menu::draw(target, x, y + 40);
}

int ConfirmMenu::getSum() const
{
  Item& item = item_ref(m_itemName);

  int sum;

  if (m_type == BUY)
  {
    sum = item.cost * m_quantity;
  }
  else
  {
    sum = item.cost * m_quantity / 2;
  }

  return sum;
}

///////////////////////////////////////////////////////////////////////////////

ShopMenu::ShopMenu(const std::vector<std::string>& inventory)
 : m_buyMenu(0),
   m_sellMenu(0),
   m_inventory(inventory)
{
  addEntry("Buy");
  addEntry("Sell");
  addEntry("Leave");

  setVisible(true);
}

ShopMenu::~ShopMenu()
{
  delete m_buyMenu;
  delete m_sellMenu;
}

void ShopMenu::moveArrow(Direction dir)
{
  if (m_buyMenu)
  {
    m_buyMenu->moveArrow(dir);
  }
  else if (m_sellMenu)
  {
    m_sellMenu->moveArrow(dir);
  }
  else
  {
    Menu::moveArrow(dir);
  }
}

void ShopMenu::handleConfirm()
{
  std::string choice = getCurrentMenuChoice();

  if (m_buyMenu)
  {
    m_buyMenu->handleConfirm();
  }
  else if (m_sellMenu)
  {
    m_sellMenu->handleConfirm();

    // If the player runs out of items.
    if (!m_sellMenu->isVisible())
    {
      delete m_sellMenu;
      m_sellMenu = 0;
    }
  }
  else
  {
    if (choice == "Buy")
    {
      m_buyMenu = new ShopBuyMenu(m_inventory);
      m_buyMenu->setVisible(true);
    }
    else if (choice == "Sell")
    {
      if (!get_player()->getInventory().empty())
      {
        m_sellMenu = new ShopSellMenu;
        m_sellMenu->setVisible(true);
      }
      else
      {
        play_sound(config::get("SOUND_CANCEL"));
      }
    }
    else if (choice == "Leave")
    {
      setVisible(false);
    }
  }
}

void ShopMenu::handleEscape()
{
  std::string choice = getCurrentMenuChoice();

  if (m_buyMenu)
  {
    m_buyMenu->handleEscape();
    if (!m_buyMenu->isVisible())
    {
      delete m_buyMenu;
      m_buyMenu = 0;
    }
  }
  else if (m_sellMenu)
  {
    m_sellMenu->handleEscape();
    if (!m_sellMenu->isVisible())
    {
      delete m_sellMenu;
      m_sellMenu = 0;
    }
  }
  else
  {
    setVisible(false);
  }
}

void ShopMenu::draw(sf::RenderTarget& target, int x, int y)
{
  draw_frame(target, 0, 0, config::GAME_RES_X, config::GAME_RES_Y);
  draw_frame(target, 0, 0, config::GAME_RES_X, 24);

  if (!m_buyMenu && !m_sellMenu)
  {
    Menu::draw(target, 0, config::GAME_RES_Y - getHeight());

    draw_text_bmp(target, 8, 8, "What can I do you for?");
  }
  else if (m_buyMenu)
  {
    m_buyMenu->draw(target, 0, 24);
  }
  else if (m_sellMenu)
  {
    m_sellMenu->draw(target, 0, 24);
  }
}

///////////////////////////////////////////////////////////////////////////////

ShopBuyMenu::ShopBuyMenu(const std::vector<std::string>& inventory)
 : m_inventory(inventory),
   m_confirmMenu(0)
{
  setMaxVisible(7);

  for (auto it = m_inventory.begin(); it != m_inventory.end(); ++it)
  {
    Item& item = item_ref(*it);

    std::string price = toString(item.cost);
    while (price.size() < 5)
      price += ' ';

    std::string entry = price + " " + item.name;
    addEntry(entry);
  }
}

ShopBuyMenu::~ShopBuyMenu()
{
  delete m_confirmMenu;
}

int ShopBuyMenu::getWidth() const
{
  return config::GAME_RES_X;
}

int ShopBuyMenu::getHeight() const
{
  return 8 * 12;
}

void ShopBuyMenu::handleConfirm()
{
  if (m_confirmMenu)
  {
    m_confirmMenu->handleConfirm();
    if (!m_confirmMenu->isVisible())
    {
      delete m_confirmMenu;
      m_confirmMenu = 0;
    }
  }
  else
  {
    Item& item = item_ref(get_string_after_first_space(getCurrentMenuChoice()));
    if (item.cost <= get_player()->getGold())
    {
      m_confirmMenu = new ConfirmMenu(ConfirmMenu::BUY, item.name);
      m_confirmMenu->setVisible(true);
    }
    else
    {
      play_sound(config::get("SOUND_CANCEL"));
    }
  }
}

void ShopBuyMenu::handleEscape()
{
  if (m_confirmMenu)
  {
    delete m_confirmMenu;
    m_confirmMenu = 0;
  }
  else
  {
    Menu::handleEscape();
  }
}

void ShopBuyMenu::moveArrow(Direction dir)
{
  if (m_confirmMenu)
  {
    m_confirmMenu->moveArrow(dir);
  }
  else
  {
    Menu::moveArrow(dir);
  }
}

void ShopBuyMenu::draw(sf::RenderTarget& target, int x, int y)
{
  std::string itemName = get_string_after_first_space(getCurrentMenuChoice());
  Item& item = item_ref(itemName);
  draw_text_bmp(target, 8, 8, "%s", item.description.c_str());

  std::vector<PlayerCharacter*> party = get_player()->getParty();
  std::vector<Entity*> playerEntites = get_player()->getTrain();

  for (size_t i = 0; i < party.size(); i++)
  {
    int posX = 8 + (i * (config::TILE_W + 48));
    int posY = y + getHeight() + 8;

    Sprite* sprite = playerEntites[i]->sprite()->clone();
    sprite->setFrame(0);
    sprite->setDirection(DIR_DOWN);

    sf::Color color = sf::Color::White;
    if (!party[i]->canEquip(itemName))
    {
      color = sf::Color(127, 127, 127);
    }

    sprite->render_ex(target, posX, posY, color);
    delete sprite;

    drawDeltas(target, party[i], itemName, posX, posY + 8 + config::TILE_H);
  }

  Menu::draw(target, x, y);

  draw_frame(target, 0, config::GAME_RES_Y - 16, config::GAME_RES_X, 16);
  draw_text_bmp(target, 8, config::GAME_RES_Y - 12, "Gold %d", get_player()->getGold());

  if (m_confirmMenu)
  {
    m_confirmMenu->draw(target,
        config::GAME_RES_X / 2 - m_confirmMenu->getWidth() / 2,
        config::GAME_RES_Y / 2 - m_confirmMenu->getHeight() / 2);
  }
}

void ShopBuyMenu::drawDeltas(sf::RenderTarget& target, PlayerCharacter* character, const std::string& itemName, int x, int y)
{
  int newStr;
  int newPow;
  int newDef;
  int newMag;
  int newMdf;
  int newSpd;

  if (character->canEquip(itemName))
  {
    Item& item = item_ref(itemName);
    std::string equipSlot = equip_type_string(item.type);

    std::string currentEquip = character->getEquipment(equipSlot) ?
        character->getEquipment(equipSlot)->name :
        "";

    character->equip(equipSlot, itemName);

    newStr = character->computeCurrentAttribute("strength");
    newPow = character->computeCurrentAttribute("power");
    newDef = character->computeCurrentAttribute("defense");
    newMag = character->computeCurrentAttribute("magic");
    newMdf = character->computeCurrentAttribute("mag.def");
    newSpd = character->computeCurrentAttribute("speed");

    character->equip(equipSlot, currentEquip);
  }
  else
  {
    newStr = character->computeCurrentAttribute("strength");
    newPow = character->computeCurrentAttribute("power");
    newDef = character->computeCurrentAttribute("defense");
    newMag = character->computeCurrentAttribute("magic");
    newMdf = character->computeCurrentAttribute("mag.def");
    newSpd = character->computeCurrentAttribute("speed");
  }

  draw_text_bmp(target, x, y,      "St%s%d",
      newStr > character->computeCurrentAttribute("strength") ? ">" :
          newStr < character->computeCurrentAttribute("strength") ? "<" : "=",
              newStr);
  draw_text_bmp(target, x, y + 12, "Po%s%d",
      newPow > character->computeCurrentAttribute("power") ? ">" :
          newPow < character->computeCurrentAttribute("power") ? "<" : "=",
      newPow);
  draw_text_bmp(target, x, y + 24, "Df%s%d",
      newDef > character->computeCurrentAttribute("defense") ? ">" :
          newDef < character->computeCurrentAttribute("defense") ? "<" : "=",
      newDef);
  draw_text_bmp(target, x, y + 36, "Mg%s%d",
      newMag > character->computeCurrentAttribute("magic") ? ">" :
          newMag < character->computeCurrentAttribute("magic") ? "<" : "=",
      newMag);
  draw_text_bmp(target, x, y + 48, "Md%s%d",
      newMdf > character->computeCurrentAttribute("mag.def") ? ">" :
          newMdf < character->computeCurrentAttribute("mag.def") ? "<" : "=",
      newMdf);
  draw_text_bmp(target, x, y + 60, "Sp%s%d",
      newSpd > character->computeCurrentAttribute("speed") ? ">" :
          newSpd < character->computeCurrentAttribute("speed") ? "<" : "=",
      newSpd);
}

///////////////////////////////////////////////////////////////////////////////

ShopSellMenu::ShopSellMenu()
 : m_confirmMenu(0)
{
  refresh();
}

ShopSellMenu::~ShopSellMenu()
{
  delete m_confirmMenu;
}

void ShopSellMenu::refresh()
{
  clear();

  setMaxVisible(7);

  for (auto it = get_player()->getInventory().begin(); it != get_player()->getInventory().end(); ++it)
  {
    std::string itemName = it->name;

    std::string stack = toString(it->stackSize);
    while (stack.size() < 2)
      stack += ' ';

    std::string entry = stack + " " + itemName;
    addEntry(entry);
  }

  if (get_player()->getInventory().empty())
  {
    setVisible(false);
  }
  else
  {
    // Fix when selling last entry.
    if (getCurrentChoiceIndex() >= getNumberOfChoice())
    {
      setCurrentChoice(getNumberOfChoice() - 1);
    }
  }
}

int ShopSellMenu::getWidth() const
{
  return config::GAME_RES_X;
}

int ShopSellMenu::getHeight() const
{
  return 8 * 12;
}

void ShopSellMenu::handleConfirm()
{
  if (m_confirmMenu)
  {
    m_confirmMenu->handleConfirm();
    if (!m_confirmMenu->isVisible())
    {
      delete m_confirmMenu;
      m_confirmMenu = 0;
    }

    refresh();
  }
  else if (isVisible())
  {
    Item& item = item_ref(get_string_after_first_space(getCurrentMenuChoice()));
    if (item.cost > 0)
    {
      m_confirmMenu = new ConfirmMenu(ConfirmMenu::SELL, item.name);
      m_confirmMenu->setVisible(true);
    }
    else
    {
      play_sound(config::get("SOUND_CANCEL"));
    }
  }
}

void ShopSellMenu::handleEscape()
{
  if (m_confirmMenu)
  {
    delete m_confirmMenu;
    m_confirmMenu = 0;
  }
  else
  {
    Menu::handleEscape();
  }
}

void ShopSellMenu::moveArrow(Direction dir)
{
  if (m_confirmMenu)
  {
    m_confirmMenu->moveArrow(dir);
  }
  else
  {
    Menu::moveArrow(dir);
  }
}

void ShopSellMenu::draw(sf::RenderTarget& target, int x, int y)
{
  std::string itemName = get_string_after_first_space(getCurrentMenuChoice());
  Item& item = item_ref(itemName);
  draw_text_bmp(target, 8, 8, "%s", item.description.c_str());

  Menu::draw(target, x, y);

  draw_frame(target, 0, config::GAME_RES_Y - 16, config::GAME_RES_X, 16);
  draw_text_bmp(target, 8, config::GAME_RES_Y - 12, "Gold %d", get_player()->getGold());

  if (m_confirmMenu)
  {
    m_confirmMenu->draw(target,
        config::GAME_RES_X / 2 - m_confirmMenu->getWidth() / 2,
        config::GAME_RES_Y / 2 - m_confirmMenu->getHeight() / 2);
  }
}

///////////////////////////////////////////////////////////////////////////////

Shop::Shop(const std::vector<std::string>& items)
 : m_menu(items)
{

}

void Shop::update()
{
  if (!m_menu.isVisible())
  {
    close();
  }
}

void Shop::draw(sf::RenderTarget& target)
{
  m_menu.draw(target, 0, 0);
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
  if (key == sf::Keyboard::Space)
  {
    m_menu.handleConfirm();
  }
  else if (key == sf::Keyboard::Escape)
  {
    m_menu.handleEscape();
  }
  else if (key == sf::Keyboard::Down) m_menu.moveArrow(DIR_DOWN);
  else if (key == sf::Keyboard::Up) m_menu.moveArrow(DIR_UP);
  else if (key == sf::Keyboard::Right) m_menu.moveArrow(DIR_RIGHT);
  else if (key == sf::Keyboard::Left) m_menu.moveArrow(DIR_LEFT);
}
