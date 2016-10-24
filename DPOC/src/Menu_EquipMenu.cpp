#include <functional>

#include "Item.h"
#include "Sound.h"
#include "Frame.h"
#include "Config.h"
#include "Player.h"
#include "Utility.h"
#include "GuiStack.h"
#include "draw_text.h"
#include "Vocabulary.h"
#include "Menu_EquipItemMenu.h"
#include "Menu_EquipMenu.h"

EquipMenu::EquipMenu(PlayerCharacter* character, int x, int y)
  : m_x(x),
    m_y(y),
    m_character(character),
    m_range(0, get_equip_names().size(), get_equip_names().size()),
    m_equipItemMenu(nullptr)
{
  for (const auto equipName : get_equip_names())
  {
    m_equipNames.push_back(equipName);
  }
}

void EquipMenu::start()
{
  using namespace std::placeholders;

  const int itemMenuX = 0;
  const int itemMenuY = 112;
  const int itemMenuW = 256;
  const int itemMenuH = 104;

  m_equipItemMenu = getGuiStack()->addWidget<EquipItemMenu>(m_character,
    std::bind(&EquipMenu::handleSelectEquipmentCallback, this, _1),
    itemMenuX, itemMenuY, itemMenuW, itemMenuH);
  m_equipItemMenu->setCursorVisible(false);
  refreshItemMenu();

  getGuiStack()->bringToFront(this);
}

bool EquipMenu::handleInput(sf::Keyboard::Key key)
{
  switch (key)
  {
  case sf::Keyboard::Up:
    m_range.subIndex(1, Range::WRAP);
    refreshItemMenu();
    break;
  case sf::Keyboard::Down:
    m_range.addIndex(1, Range::WRAP);
    refreshItemMenu();
    break;
  case sf::Keyboard::Space:
  case sf::Keyboard::Return:
    m_equipItemMenu->setCursorVisible(true);
    getGuiStack()->bringToFront(m_equipItemMenu);
    break;
  case sf::Keyboard::Escape:
    m_equipItemMenu->close();
    close();
    break;
  default:
    break;
  }

  return true;
}

void EquipMenu::draw(sf::RenderTarget& target)
{
  const int nameFrameHeight = 32;

  const int leftPanelY = m_y + nameFrameHeight;
  const int leftPanelW = 128;
  const int leftPanelH = 80;
  const int rightPanelW = 128;

  draw_frame(target, m_x, leftPanelY, leftPanelW, leftPanelH);
  draw_frame(target, m_x + leftPanelW, leftPanelY, rightPanelW, leftPanelH);

  // Top.
  draw_frame(target, m_x, m_y, config::GAME_RES_X, nameFrameHeight);
  m_character->draw(target, m_x, m_y);
  draw_text_bmp(target, m_x + 36, m_y + 8, "%s", m_character->getName().c_str());

  int offX = m_x + 8;
  int offY = m_y + 38;

  drawDeltas(target, offX, offY);

  offX = m_x + 136;

  for (size_t i = 0; i < m_equipNames.size(); i++)
  {
    Item* equipment = m_character->getEquipment(m_equipNames[i]);

    std::string typeShortName = vocab_short(m_equipNames[i]);
    std::string itemShortName = equipment ?
        limit_string(equipment->name, 8) :
        "";

    draw_text_bmp(target, offX, offY + 12 * i, "%s: %s", typeShortName.c_str(), itemShortName.c_str());
  }

  sf::RectangleShape rect = make_select_rect(offX - 1, offY - 1 + m_range.getIndex() * ENTRY_OFFSET, 114, 10);
  target.draw(rect);
}

void EquipMenu::drawDeltas(sf::RenderTarget& target, int x, int y)
{
  int newStr;
  int newLuk;
  int newDef;
  int newMag;
  int newMdf;
  int newSpd;

  if (getGuiStack()->getTop() == m_equipItemMenu)
  {
    std::string currentEquip = m_character->getEquipment(getCurrentEquipType()) ?
        m_character->getEquipment(getCurrentEquipType())->name :
        "";

    if (m_equipItemMenu->getSelectedItemName() != EquipItemMenu::removeEquipmentString())
    {
      m_character->equip(getCurrentEquipType(), m_equipItemMenu->getSelectedItemName());
    }
    else
    {
      m_character->equip(getCurrentEquipType(), "");
    }

    newStr = m_character->computeCurrentAttribute(terms::strength);
    newDef = m_character->computeCurrentAttribute(terms::defense);
    newMag = m_character->computeCurrentAttribute(terms::magic);
    newMdf = m_character->computeCurrentAttribute(terms::magdef);
    newSpd = m_character->computeCurrentAttribute(terms::speed);
    newLuk = m_character->computeCurrentAttribute(terms::luck);

    m_character->equip(getCurrentEquipType(), currentEquip);
  }
  else
  {
    newStr = m_character->computeCurrentAttribute(terms::strength);
    newDef = m_character->computeCurrentAttribute(terms::defense);
    newMag = m_character->computeCurrentAttribute(terms::magic);
    newMdf = m_character->computeCurrentAttribute(terms::magdef);
    newSpd = m_character->computeCurrentAttribute(terms::speed);
    newLuk = m_character->computeCurrentAttribute(terms::luck);
  }

  draw_text_bmp(target, x, y,      "%s: %d (%d)", vocab_mid(terms::strength).c_str(), m_character->computeCurrentAttribute(terms::strength), newStr);
  draw_text_bmp(target, x, y + 12, "%s: %d (%d)", vocab_mid(terms::defense).c_str(), m_character->computeCurrentAttribute(terms::defense), newDef);
  draw_text_bmp(target, x, y + 24, "%s: %d (%d)", vocab_mid(terms::magic).c_str(), m_character->computeCurrentAttribute(terms::magic), newMag);
  draw_text_bmp(target, x, y + 36, "%s: %d (%d)", vocab_mid(terms::magdef).c_str(), m_character->computeCurrentAttribute(terms::magdef), newMdf);
  draw_text_bmp(target, x, y + 48, "%s: %d (%d)", vocab_mid(terms::speed).c_str(), m_character->computeCurrentAttribute(terms::speed), newSpd);
  draw_text_bmp(target, x, y + 60, "%s: %d (%d)", vocab_mid(terms::luck).c_str(), m_character->computeCurrentAttribute(terms::luck), newLuk);
}

std::string EquipMenu::getCurrentEquipType() const
{
  return m_equipNames[m_range.getIndex()];
}

void EquipMenu::handleSelectEquipmentCallback(const std::string& itemName)
{
  if (itemName != EquipItemMenu::removeEquipmentString())
  {
    doEquip(itemName);
  }
  else
  {
    doUnEquip();
  }
}

void EquipMenu::doEquip(const std::string& itemName)
{
  Item* currentItem = get_player()->getItem(itemName);
  Item* currentEquip = m_character->getEquipment(getCurrentEquipType());

  if (currentItem && getCurrentEquipType() == equip_type_string(currentItem->type) && m_character->canEquip(*currentItem))
  {
    if (currentEquip)
    {
      get_player()->addItemToInventory(currentEquip->name, 1);
    }

    m_character->equip(getCurrentEquipType(), currentItem->name);
    get_player()->removeItemFromInventory(currentItem->name, 1);

    refreshItemMenu();
    m_equipItemMenu->setCursorVisible(false);
    getGuiStack()->bringToFront(this);

    play_sound(config::get("SOUND_EQUIP"));
  }
  else
  {
    play_sound(config::get("SOUND_CANCEL"));
  }
}

void EquipMenu::doUnEquip()
{
  Item* currentEquip = m_character->getEquipment(getCurrentEquipType());

  if (currentEquip)
  {
    get_player()->addItemToInventory(currentEquip->name, 1);
    m_character->equip(getCurrentEquipType(), "");

    refreshItemMenu();
    m_equipItemMenu->setCursorVisible(false);
    getGuiStack()->bringToFront(this);

    play_sound(config::get("SOUND_EQUIP"));
  }
  else
  {
    play_sound(config::get("SOUND_CANCEL"));
  }
}

void EquipMenu::refreshItemMenu()
{
  m_equipItemMenu->setEquipmentType(getCurrentEquipType());
  m_equipItemMenu->refresh();
}
