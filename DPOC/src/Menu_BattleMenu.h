#ifndef MENU_BATTLEMENU_H_
#define MENU_BATTLEMENU_H_

#include <map>
#include <string>
#include <vector>

#include "Target.h"
#include "GuiWidget.h"

class Battle;
class BattleActionMenu;
class BattleMonsterMenu;
class BattleStatusMenu;

struct Spell;
class Character;
class PlayerCharacter;

class BattleMenu : public GuiWidget
{
public:
  BattleMenu(Battle* battle, const std::vector<Character*>& monsters);
  ~BattleMenu();

  void start() override;
  bool handleInput(sf::Keyboard::Key key) override;
  void draw(sf::RenderTarget& target) override;

  void activate();
  void resetChoice();

  void setActionMenuHidden(bool hidden);

  void addMonster(Character* monster);
private:
  void monsterSelected(Character* monster);
  void playerSelected(PlayerCharacter* character);
  void statusMenuEscape();
  void battleActionSelected(const std::string& action);
  void battleActionEscape();

  void itemSelected(const std::string& itemName);
  void spellSelected(const Spell* spell);

  void nextActor();

  void prepareAction();

  void selectMonster();
  void selectCharacter();

  void closeSpellMenu();
  void closeItemMenu();

  Character* getTarget(Target targetType) const;
  void reshowSpellOrItemMenu();
private:
  Battle* m_battle;
  std::vector<Character*> m_monsters;

  bool m_actionMenuHidden;

  BattleActionMenu* m_actionMenu = nullptr;
  BattleMonsterMenu* m_monsterMenu = nullptr;
  BattleStatusMenu* m_statusMenu = nullptr;

  // Remember the last selected spell for characters during the battle.
  std::map<Character*, int> m_spellMemory;
  std::map<Character*, int> m_itemMemory;
};

#endif /* MENU_BATTLEMENU_H_ */
