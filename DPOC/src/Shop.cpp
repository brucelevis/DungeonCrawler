#include "Frame.h"
#include "draw_text.h"
#include "Config.h"
#include "Utility.h"
#include "Sound.h"
#include "Vocabulary.h"

#include "PlayerCharacter.h"
#include "Player.h"
#include "Entity.h"
#include "Sprite.h"
#include "Direction.h"

#include "Item.h"

#include "Shop.h"

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

void ShopMenu::draw(sf::RenderTarget& target, int, int)
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

  static int bump = 0;
  static int bumpCount = 10;
  if (bumpCount > 0)
  {
    bumpCount--;
    if (bumpCount == 0)
    {
      bump = !bump;
      bumpCount = 10;
    }
  }

  for (size_t i = 0; i < party.size(); i++)
  {
    const sf::Texture* texture = party[i]->getTexture();
    sf::Sprite sprite;
    sprite.setTexture(*texture);
    sprite.setScale(0.5, 0.5);

    int posX = 8 + (i * (sprite.getGlobalBounds().width + 48));
    int posY = y + getHeight() + 4;

    sf::Color color = sf::Color::White;
    int _bump = 0;
    if (!party[i]->canEquip(item) && (item.type != ITEM_USE && item.type != ITEM_USE_BATTLE && item.type != ITEM_USE_MENU))
    {
      color = sf::Color(127, 127, 127);
    }
    else
    {
      _bump = bump;
    }

    sprite.setPosition(posX, posY - _bump);
    sprite.setColor(color);
    target.draw(sprite);

    drawDeltas(target, party[i], itemName, posX, posY + 4 + sprite.getGlobalBounds().height);
  }

  Menu::draw(target, x, y);

  draw_frame(target, 0, config::GAME_RES_Y - 16, config::GAME_RES_X, 16);
  draw_text_bmp(target, 8, config::GAME_RES_Y - 12, "%s %d", vocab_upcase(terms::gold).c_str(), get_player()->getGold());

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
  int newDef;
  int newMag;
  int newMdf;
  int newSpd;
  int newLuk;

  Item& item = item_ref(itemName);

  if (character->canEquip(item))
  {
    std::string equipSlot = equip_type_string(item.type);

    std::string currentEquip = character->getEquipment(equipSlot) ?
        character->getEquipment(equipSlot)->name :
        "";

    character->equip(equipSlot, itemName);

    newStr = character->computeCurrentAttribute(terms::strength);
    newDef = character->computeCurrentAttribute(terms::defense);
    newMag = character->computeCurrentAttribute(terms::magic);
    newMdf = character->computeCurrentAttribute(terms::magdef);
    newSpd = character->computeCurrentAttribute(terms::speed);
    newLuk = character->computeCurrentAttribute(terms::luck);

    character->equip(equipSlot, currentEquip);
  }
  else
  {
    newStr = character->computeCurrentAttribute(terms::strength);
    newDef = character->computeCurrentAttribute(terms::defense);
    newMag = character->computeCurrentAttribute(terms::magic);
    newMdf = character->computeCurrentAttribute(terms::magdef);
    newSpd = character->computeCurrentAttribute(terms::speed);
    newLuk = character->computeCurrentAttribute(terms::luck);
  }

  draw_text_bmp(target, x, y,      "%s%s%d", vocab_short(terms::strength).c_str(),
      newStr > character->computeCurrentAttribute(terms::strength) ? ">" :
          newStr < character->computeCurrentAttribute(terms::strength) ? "<" : "=",
              newStr);
  draw_text_bmp(target, x, y + 12, "%s%s%d", vocab_short(terms::defense).c_str(),
      newDef > character->computeCurrentAttribute(terms::defense) ? ">" :
          newDef < character->computeCurrentAttribute(terms::defense) ? "<" : "=",
      newDef);
  draw_text_bmp(target, x, y + 24, "%s%s%d", vocab_short(terms::magic).c_str(),
      newMag > character->computeCurrentAttribute(terms::magic) ? ">" :
          newMag < character->computeCurrentAttribute(terms::magic) ? "<" : "=",
      newMag);
  draw_text_bmp(target, x, y + 36, "%s%s%d", vocab_short(terms::magdef).c_str(),
      newMdf > character->computeCurrentAttribute(terms::magdef) ? ">" :
          newMdf < character->computeCurrentAttribute(terms::magdef) ? "<" : "=",
      newMdf);
  draw_text_bmp(target, x, y + 48, "%s%s%d", vocab_short(terms::speed).c_str(),
      newSpd > character->computeCurrentAttribute(terms::speed) ? ">" :
          newSpd < character->computeCurrentAttribute(terms::speed) ? "<" : "=",
      newSpd);
  draw_text_bmp(target, x, y + 60, "%s%s%d", vocab_short(terms::luck).c_str(),
      newLuk > character->computeCurrentAttribute(terms::luck) ? ">" :
          newLuk < character->computeCurrentAttribute(terms::luck) ? "<" : "=",
      newLuk);
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
  draw_text_bmp(target, 8, config::GAME_RES_Y - 12, "%s %d", vocab_upcase(terms::gold).c_str(), get_player()->getGold());

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
