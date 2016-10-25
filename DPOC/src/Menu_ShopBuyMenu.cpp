#include "Item.h"
#include "draw_text.h"
#include "Config.h"
#include "Utility.h"
#include "Player.h"
#include "Sound.h"
#include "Vocabulary.h"

#include "Menu_ShopConfirmMenu.h"
#include "Menu_ShopBuyMenu.h"

ShopBuyMenu::ShopBuyMenu(const std::vector<std::string>& inventory)
 : m_inventory(inventory),
   m_presenter(MenuPresenter::NO_STYLE)
{
  for (auto it = m_inventory.begin(); it != m_inventory.end(); ++it)
  {
    Item& item = item_ref(*it);

    std::string price = toString(item.cost);
    while (price.size() < 5)
    {
      price += ' ';
    }

    std::string entry = price + " " + item.name;
    m_presenter.addEntry(entry);
  }

  m_presenter.setMaxVisible(7);
}

int ShopBuyMenu::getWidth() const
{
  return config::GAME_RES_X;
}

int ShopBuyMenu::getHeight() const
{
  return 8 * ENTRY_OFFSET;
}

bool ShopBuyMenu::handleInput(sf::Keyboard::Key key)
{
  switch (key)
  {
  case sf::Keyboard::Up:
    m_presenter.scrollUp();
    break;
  case sf::Keyboard::Down:
    m_presenter.scrollDown();
    break;
  case sf::Keyboard::Space:
  case sf::Keyboard::Return:
    handleConfirm();
    break;
  case sf::Keyboard::Escape:
    handleEscape();
    break;
  default:
    break;
  }

  return true;
}

void ShopBuyMenu::draw(sf::RenderTarget& target)
{
  const int x = 0;
  const int y = 24;

  std::string itemName = get_string_after_first_space(m_presenter.getSelectedOption().entryName);
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
    int bump_ = 0;
    if (!party[i]->canEquip(item) && (item.type != ITEM_USE && item.type != ITEM_USE_BATTLE && item.type != ITEM_USE_MENU))
    {
      color = sf::Color(127, 127, 127);
    }
    else
    {
      bump_ = bump;
    }

    sprite.setPosition(posX, posY - bump_);
    sprite.setColor(color);
    target.draw(sprite);

    drawDeltas(target, party[i], itemName, posX, posY + 4 + sprite.getGlobalBounds().height);
  }

  draw_frame(target, x, y, getWidth(), getHeight());
  m_presenter.draw(target, x, y, this);

  draw_frame(target, 0, config::GAME_RES_Y - 16, config::GAME_RES_X, 16);
  draw_text_bmp(target, 8, config::GAME_RES_Y - 12, "%s %d", vocab_upcase(terms::gold).c_str(), get_player()->getGold());
}

void ShopBuyMenu::handleConfirm()
{
  Item& item = item_ref(get_string_after_first_space(m_presenter.getSelectedOption().entryName));
  if (item.cost <= get_player()->getGold())
  {
    getGuiStack()->addWidget<ShopConfirmMenu>(ShopConfirmMenu::BUY, item.name);
  }
  else
  {
    play_sound(config::get("SOUND_CANCEL"));
  }
}

void ShopBuyMenu::handleEscape()
{
  close();
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
