#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>
#include <stack>

#include <SFML/Graphics.hpp>

#include "Direction.h"

class Item;
class Spell;
class Character;

class Menu
{
public:
  Menu();
  virtual ~Menu();

  void setVisible(bool visible) { m_visible = visible; }
  bool isVisible() const { return m_visible; }

  virtual void handleConfirm() = 0;
  virtual void handleEscape() { setVisible(false); }

  virtual void moveArrow(Direction dir);

  virtual void draw(sf::RenderTarget& target, int x, int y);

  std::string getCurrentMenuChoice() const { return m_menuChoices[m_currentMenuChoice]; }
  std::string getChoice(size_t index) const { return m_menuChoices[index]; }

  void addEntry(const std::string& choice) { m_menuChoices.push_back(choice); }

  void clear() { m_menuChoices.clear(); }

  virtual int getWidth() const;
  virtual int getHeight() const;

  int getNumberOfChoice() const { return m_menuChoices.size(); }

  void setCursorVisible(bool visible) { m_cursorVisible = visible; }
  bool cursorVisible() const { return m_cursorVisible; }

  void resetChoice() { m_currentMenuChoice = 0; }
protected:
  void setMaxVisible(int maxVisible) { m_maxVisible = maxVisible; }
  int getCurrentChoiceIndex() const { return m_currentMenuChoice; }
  void drawSelectArrow(sf::RenderTarget& target, int x, int y);
private:
  sf::Texture* m_arrowTexture;
  bool m_visible;
  int m_currentMenuChoice;
  std::vector<std::string> m_menuChoices;

  int m_maxVisible;
  int m_scroll;

  bool m_cursorVisible;
};

class MainMenu;
class ItemMenu;
class SpellMenu;
class CharacterMenu;
class EquipMenu;

class MainMenu : public Menu
{
  enum State
  {
    STATE_MAIN_MENU,
    STATE_ITEM_MENU,
    STATE_CHARACTER_MENU,
    STATE_EQUIP_MENU,
    STATE_SPELL_MENU,
    STATE_STATUS_MENU,
    STATE_SAVE_MENU
  };
public:
  MainMenu();
  ~MainMenu();

  void handleConfirm();
  void handleEscape();

  void moveArrow(Direction dir);

  void open();

  void draw(sf::RenderTarget& target, int x, int y);
private:
  void openItemMenu();
  void closeItemMenu();

  void openSpellMenu(const std::string& characterName);
  void closeSpellMenu();

  void openCharacterMenu();
  void closeCharacterMenu();

  void openEquipMenu();
  void closeEquipMenu();

  void drawStatus(sf::RenderTarget& target, int x, int y);
private:
  ItemMenu* m_itemMenu;
  SpellMenu* m_spellMenu;
  CharacterMenu* m_characterMenu;
  EquipMenu* m_equipMenu;

  std::stack<State> m_stateStack;
};

class ItemMenu : public Menu
{
public:
  ItemMenu();
  ItemMenu(int width, int height);
  virtual ~ItemMenu() {}

  void handleConfirm();

  void refresh();

  void draw(sf::RenderTarget& target, int x, int y);

  int getWidth() const;
  int getHeight() const;

  std::string getSelectedItemName() const;
private:
  bool hasItem(const std::string& name) const;
  const Item* getItem(const std::string& name) const;
protected:
  std::vector<const Item*> m_items;

  int m_width, m_height;
};

class EquipItemMenu : public ItemMenu
{
public:
  EquipItemMenu(int width, int height);
  void refresh(const std::string& equipmentType);

  bool validChoice() const { return getCurrentMenuChoice() != "* Remove *"; }
};

class SpellMenu : public Menu
{
public:
  SpellMenu(const std::string& characterName);

  void handleConfirm();

  const Spell* getSelectedSpell() const;

  void draw(sf::RenderTarget& target, int x, int y);

  int getWidth() const;
  int getHeight() const;
private:
};

class CharacterMenu : public Menu
{
public:
  CharacterMenu();

  void handleConfirm();

  int getWidth() const;
  int getHeight() const;

  void refresh();

  void draw(sf::RenderTarget& target, int x, int y);

  void setSpellToUse(const Spell* spell) { m_spellToUse = spell; }
  const Spell* getSpellToUse() const { return m_spellToUse; }

  void setItemToUse(const std::string& itemToUse) { m_itemToUse = itemToUse; }
  std::string getItemToUse() const { return m_itemToUse; }

  Character* getUser() const { return m_user; }
  Character* getTarget() const { return m_target; }

  void setUserToCurrentChoice();
  void setTargetToCurrentChoice();
private:
  const Spell* m_spellToUse;
  std::string m_itemToUse;

  Character* m_user;
  Character* m_target;
};

class EquipMenu : public Menu
{
  enum State
  {
    STATE_SELECT_EQUIPMENT_TYPE,
    STATE_EQUIP_ITEM
  };
public:
  EquipMenu(Character* character);
  ~EquipMenu();

  void handleConfirm();
  void handleEscape();

  void moveArrow(Direction dir);

  void draw(sf::RenderTarget& target, int x, int y);
private:
  void doEquip();
  void doUnEquip();

  void drawDeltas(sf::RenderTarget& target, int x, int y);
private:
  Character* m_character;
  EquipItemMenu* m_itemMenu;

  State m_state;
};

class BattleActionMenu;
class BattleStatusMenu;
class BattleMonsterMenu;

class BattleMenu : public Menu
{
  enum State
  {
    STATE_SELECT_ACTION,
    STATE_SELECT_CHARACTER,
    STATE_SELECT_MONSTER
  };
public:
  BattleMenu(const std::vector<Character*>& monsters);
  ~BattleMenu();

  void handleConfirm();
  void handleEscape();
  void moveArrow(Direction dir);

  void draw(sf::RenderTarget& target, int x, int y);
private:
  BattleActionMenu* m_actionMenu;
  BattleStatusMenu* m_statusMenu;
  BattleMonsterMenu* m_monsterMenu;

  std::stack<State> m_stateStack;
};

class BattleActionMenu : public Menu
{
public:
  BattleActionMenu();

  void handleConfirm();

//  void draw(sf::RenderTarget& target, int x, int y);
private:
};

class BattleStatusMenu : public Menu
{
public:
  BattleStatusMenu();

  void handleConfirm();

  void draw(sf::RenderTarget& target, int x, int y);

  int getWidth() const;

  void prevActor();
  void nextActor();
private:
  int m_currentActor;
};

class BattleMonsterMenu : public Menu
{
public:
  BattleMonsterMenu(const std::vector<Character*>& monsters);

  void handleConfirm();
  void moveArrow(Direction dir);

  void draw(sf::RenderTarget& target, int x, int y);
private:
  std::vector<Character*> m_monsters;
};

#endif
