#include <functional>

#include "Config.h"
#include "Error.h"
#include "Game.h"
#include "logger.h"
#include "Sound.h"
#include "Frame.h"
#include "Player.h"
#include "Vocabulary.h"
#include "Target.h"

#include "SaveMenu.h"
#include "Menu_ItemMenu.h"
#include "Menu_SpellMenu.h"
#include "Menu_EquipMenu.h"
#include "Menu_CharacterMenu.h"

#include "Menu_MainMenu.h"

MainMenu::MainMenu()
  : m_characterMenu(nullptr),
    m_state(STATE_DEFAULT)
{
  addEntry("Item");
  addEntry("Spell");
  addEntry("Equip");
  addEntry("Status");
  addEntry("Skills");
  addEntry("Map");
  addEntry("Save");
  addEntry("Close");

  m_range = Range{0, m_options.size(), m_options.size()};
}

MainMenu::~MainMenu()
{
  getGuiStack()->removeWidget(m_characterMenu);
}

void MainMenu::start()
{
  const int characterMenuX = 72;
  const int characterMenuY = 0;

  m_characterMenu = getGuiStack()->addWidget<CharacterMenu>(
    std::bind(&MainMenu::characterSelected, this, std::placeholders::_1),
    std::bind(&MainMenu::escapeFromCharacterMenu, this),
    characterMenuX, characterMenuY);

  m_characterMenu->setCursorVisible(false);
  getGuiStack()->bringToFront(this);
}

bool MainMenu::handleInput(sf::Keyboard::Key key)
{
  switch (key)
  {
  case sf::Keyboard::Up:
    m_range.subIndex(1, Range::WRAP);
    break;
  case sf::Keyboard::Down:
    m_range.addIndex(1, Range::WRAP);
    break;
  case sf::Keyboard::Space:
  case sf::Keyboard::Return:
    handleConfirm();
    break;
  case sf::Keyboard::Escape:
    getGuiStack()->removeWidget(this);
    break;
  default:
    break;
  }

  return true;
}

void MainMenu::draw(sf::RenderTarget& target)
{
  const int menuChoicesWidth = 72;
  const int gpPanelHeight = 32;

  const int x = 0;
  const int y = 0;

  draw_frame(target, x, y, config::GAME_RES_X, config::GAME_RES_Y);

  draw_frame(target, x, y, menuChoicesWidth, m_options.size() * 16);
  draw_frame(target, x, y + 208, menuChoicesWidth, gpPanelHeight);

  draw_text_bmp(target, x + 8, y + 13*16+7, "%s", vocab_short(terms::gold).c_str());
  draw_text_bmp(target, x + 8, y + 13*16+19, "%d", get_player()->getGold());

  for (size_t i = 0; i < m_options.size(); i++)
  {
    draw_text_bmp(target, x + 18, y + 10 + i * 14, "%s", m_options[i].c_str());
    if (i == m_range.getIndex())
    {
      drawSelectArrow(target, x + 8, y + 10 + i * 14);
    }
  }
}

void MainMenu::handleConfirm()
{
  std::string option = m_options[m_range.getIndex()];

  if (option == "Close")
  {
    getGuiStack()->removeWidget(this);
  }
  else if (option == "Item")
  {
    getGuiStack()->addWidget<ItemMenu>(std::bind(&MainMenu::itemSelected, this, std::placeholders::_1), 16, 16);
  }
  else if (option == "Spell")
  {
    activateCharacterMenu(STATE_SELECT_SPELL);
  }
  else if (option == "Status")
  {
    activateCharacterMenu(STATE_VIEW_STATUS);
  }
  else if (option == "Skills")
  {
    activateCharacterMenu(STATE_VIEW_SKILLS);
  }
  else if (option == "Equip")
  {
    activateCharacterMenu(STATE_EQUIP);
  }
  else if (option == "Save")
  {
    getGuiStack()->addWidget<SaveMenu>(SaveMenu::SAVE);
  }
  else if (option == "Map")
  {
    Game::instance().openMap();
  }
}

void MainMenu::addEntry(const std::string& entry)
{
  m_options.push_back(entry);
}

void MainMenu::activateCharacterMenu(State state)
{
  m_state = state;

  m_characterMenu->reset();
  m_characterMenu->setCursorVisible(true);
  getGuiStack()->bringToFront(m_characterMenu);
}

void MainMenu::characterSelected(PlayerCharacter* character)
{
  switch (m_state)
  {
  case STATE_DEFAULT:
    TRACE("ERROR: %s in STATE_DEFAULT should never happen!", __func__);
    break;
  case STATE_SELECT_SPELL:
    if (character && !character->hasStatus("Dead"))
    {
      m_characterMenu->setUser(character);
      getGuiStack()->addWidget<SpellMenu>(std::bind(&MainMenu::spellSelected, this, std::placeholders::_1), character->getName(), 16, 16);
    }
    else
    {
      play_sound(config::get("SOUND_CANCEL"));
    }
    break;
  case STATE_VIEW_STATUS:
    CRASH("STATE_VIEW_STATUS not done yet.");
    break;
  case STATE_VIEW_SKILLS:
    CRASH("STATE_VIEW_SKILLS not done yet.");
    break;
  case STATE_EQUIP:
    getGuiStack()->addWidget<EquipMenu>(character, 0, 0);
    break;
  case STATE_ITEM:
  {
    Item* itemToUse = get_player()->getItem(m_characterMenu->getItemToUse());

    if (m_characterMenu->getItemToUse().size() > 0 && itemToUse)
    {
      m_characterMenu->setUser(character);
      m_characterMenu->setTarget(character);

      bool targetOK = isOKTarget(m_characterMenu->getTarget(),
          create_item(m_characterMenu->getItemToUse(), 1).target);

      if (m_characterMenu->getTarget() && targetOK && m_characterMenu->getUser()->canUseItemInMenu(*itemToUse))
      {
        play_sound(config::get("SOUND_USE_ITEM"));

        use_item(itemToUse, m_characterMenu->getUser(), m_characterMenu->getTarget());

        get_player()->removeItemFromInventory(m_characterMenu->getItemToUse(), 1);

        getGuiStack()->findWidget<ItemMenu>()->refresh();
      }
      else
      {
        play_sound(config::get("SOUND_CANCEL"));
      }

      // Close if no more items.
//        if (get_player()->getItem(m_characterMenu->getItemToUse()) == 0)
//        {
//          closeCharacterMenu();
//        }
    }
    break;
  }
  case STATE_CAST_SPELL:
    if (m_characterMenu->getSpellToUse())
    {
      m_characterMenu->setTarget(character);

      bool targetOK = isOKTarget(m_characterMenu->getTarget(), m_characterMenu->getSpellToUse()->target);

      if (can_cast_spell(m_characterMenu->getSpellToUse(), m_characterMenu->getUser()) && targetOK)
      {
        play_sound(config::get("SOUND_USE_ITEM"));

        cast_spell(m_characterMenu->getSpellToUse(),
            m_characterMenu->getUser(),
            m_characterMenu->getTarget());

        // Reduce it here since cast_spell is called for each target when
        // spell has multiple targets.
        m_characterMenu->getUser()->getAttribute(terms::mp).current -= m_characterMenu->getSpellToUse()->mpCost;
      }
      else
      {
        play_sound(config::get("SOUND_CANCEL"));
      }
    }
    break;
  }
}

void MainMenu::itemSelected(const std::string& itemName)
{
  Item* item = get_player()->getItem(itemName);
  if (item && (item->type == ITEM_USE || item->type == ITEM_USE_MENU) &&
      (item->target == TARGET_SINGLE_ALLY || item->target == TARGET_DEAD))
  {
    m_characterMenu->setItemToUse(itemName);
    activateCharacterMenu(STATE_ITEM);
  }
  else
  {
    play_sound(config::get("SOUND_CANCEL"));
  }
}

void MainMenu::spellSelected(const Spell* spell)
{
  if (spell && !spell->battleOnly && can_cast_spell(spell, m_characterMenu->getUser()))
  {
    m_characterMenu->setSpellToUse(spell);
    activateCharacterMenu(STATE_CAST_SPELL);
  }
  else
  {
    play_sound(config::get("SOUND_CANCEL"));
  }
}

void MainMenu::escapeFromCharacterMenu()
{
  switch (m_state)
  {
  case STATE_DEFAULT:
    TRACE("ERROR: %s in STATE_DEFAULT should never happen!", __func__);
    break;
  case STATE_SELECT_SPELL:
  case STATE_VIEW_STATUS:
  case STATE_VIEW_SKILLS:
  case STATE_EQUIP:
    m_characterMenu->setCursorVisible(false);
    getGuiStack()->yield(m_characterMenu);
    break;
  case STATE_ITEM:
    m_characterMenu->setCursorVisible(false);
    getGuiStack()->findWidget<ItemMenu>()->setVisible(true);
    getGuiStack()->yield(m_characterMenu);  // Push below item menu
    getGuiStack()->yield(m_characterMenu);  // Push below main menu
    break;
  case STATE_CAST_SPELL:
    getGuiStack()->findWidget<SpellMenu>()->setVisible(true);
    getGuiStack()->yield(m_characterMenu);  // Push below spell menu
    m_state = STATE_SELECT_SPELL;
    break;
  }
}

// TODO: Move these to their own classes.
//void MainMenu::drawStatus(sf::RenderTarget& target, int x, int y)
//{
//  PlayerCharacter* character = Game::instance().getPlayer()->getCharacter(m_characterMenu->getCurrentMenuChoice());
//
//  draw_frame(target, 16, 16, 14*16, 13*16);
//
//  character->draw(target, x, y);
//
//  draw_text_bmp_ex(target, x + 40, y,
//      get_status_effect(character->getStatus())->color,
//      "%s (%s)", character->getName().c_str(), character->getStatus().c_str());
//
//  draw_hp(target, character, x + 40, y + 12);
//  draw_mp(target, character, x + 40, y + 24);
//
//  draw_text_bmp(target, x + 40 + 96, y + 12, "Lv: %d", character->computeCurrentAttribute(terms::level));
//  draw_text_bmp(target, x + 40 + 96, y + 24, "Tn: %d", character->toNextLevel());
//
//  y += 40;
//
//  draw_stat_block(target, character, x, y);
//
//  auto equipNames = get_equip_names();
//  for (size_t i = 0; i < equipNames.size(); i++)
//  {
//    Item* item = character->getEquipment(equipNames[i]);
//    draw_text_bmp(target, x, y + 84 + 12 * i, "%s: %s", vocab(equipNames[i]).c_str(), item ? item->name.c_str(): "");
//  }
//}
//
//void MainMenu::drawSkills(sf::RenderTarget& target, int x, int y)
//{
//  PlayerCharacter* character = Game::instance().getPlayer()->getCharacter(m_characterMenu->getCurrentMenuChoice());
//
//  int frameX = 16;
//  int frameY = 16;
//  int frameW = 14*16;
//  int frameH = 13*16;
//
//  draw_frame(target, frameX, frameY, frameW, frameH);
//
//  character->draw(target, x, y);
//
//  std::vector<std::string> skills = Skill::getAllSkills();
//
//  draw_text_bmp_ex(target, x + 40, y,
//      get_status_effect(character->getStatus())->color,
//      "%s (%s)", character->getName().c_str(), character->getStatus().c_str());
//
//  draw_hp(target, character, x + 40, y + 12);
//  draw_mp(target, character, x + 40, y + 24);
//
//  draw_text_bmp(target, x + 40 + 96, y + 12, "Lv: %d", character->computeCurrentAttribute(terms::level));
//  draw_text_bmp(target, x + 40 + 96, y + 24, "Tn: %d", character->toNextLevel());
//
//  y += 40;
//  for (size_t i = 0; i < skills.size(); i++)
//  {
//    std::string& skill = skills[i];
//    int skillPercent = character->getBaseAttribute(skill);
//
//    std::string str = toString(skillPercent) + "%";
//
//    draw_text_bmp(target, x, y + i * 12, "%s", skill.c_str(), skillPercent);
//    draw_text_bmp(target, x + frameW - str.size() * 8 - 16, y + i * 12, "%s", str.c_str());
//  }
//}
