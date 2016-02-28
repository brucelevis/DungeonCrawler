#ifndef SCRIPTMENU_H_
#define SCRIPTMENU_H_

#include <string>

#include "Scriptable.h"
#include "Menu.h"
#include "Lua.h"

class ScriptMenu : public Menu, public Scriptable
{
public:
  ScriptMenu(const std::string& scriptFile);
  ~ScriptMenu();

  void handleConfirm() override;
  void handleEscape() override;
  void moveArrow(Direction dir) override;

  void draw(sf::RenderTarget& target, int x, int y) override;

  int getWidth() const override;
  int getHeight() const override;
protected:
  void invalidate() override;
private:
  void baseDraw(sf::RenderTarget* target, int x, int y);
  void baseHandleEscape();
  void baseMoveArrow(int dir);
  int baseGetWidth() const;
  int baseGetHeight() const;
};

#endif /* SCRIPTMENU_H_ */
