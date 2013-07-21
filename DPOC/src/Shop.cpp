#include "Frame.h"
#include "draw_text.h"
#include "Config.h"
#include "Utility.h"

#include "PlayerCharacter.h"
#include "Player.h"
#include "Entity.h"
#include "Sprite.h"
#include "Direction.h"

#include "Item.h"

#include "Shop.h"

ShopMenu::ShopMenu()
 : m_buyMenu(0),
   m_sellMenu(0)
{
  addEntry("Buy");
  addEntry("Sell");
  addEntry("Leave");

  setVisible(true);

  m_inventory.push_back("Copper Sword");
  m_inventory.push_back("Herb");
  m_inventory.push_back("Magic Staff");
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

  }
  else if (m_sellMenu)
  {

  }
  else
  {
    if (choice == "Buy")
    {
      m_buyMenu = new ShopBuyMenu(m_inventory);
    }
    else if (choice == "Sell")
    {

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
    delete m_buyMenu;
    m_buyMenu = 0;
  }
  else if (m_sellMenu)
  {
    delete m_sellMenu;
    m_sellMenu = 0;
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
}

///////////////////////////////////////////////////////////////////////////////

ShopBuyMenu::ShopBuyMenu(const std::vector<std::string>& inventory)
 : m_inventory(inventory)
{
  setMaxVisible(8);

  for (auto it = m_inventory.begin(); it != m_inventory.end(); ++it)
  {
    Item& item = item_ref(*it);

    std::string price = toString(item.cost);
    while (price.size() < 5)
      price += ' ';

    std::string entry = price + item.name;
    addEntry(entry);
  }
}

int ShopBuyMenu::getWidth() const
{
  return config::GAME_RES_X;
}

int ShopBuyMenu::getHeight() const
{
  return 8 * 12;
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
  else if (key == sf::Keyboard::Down) m_menu.moveArrow(DIR_DOWN);
}
