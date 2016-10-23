#ifndef MENU_BATTLESTATUSMENU_H_
#define MENU_BATTLESTATUSMENU_H_

#include <vector>
#include <functional>

#include "GuiWidget.h"
#include "PlayerCharacter.h"

class BattleStatusMenu : public GuiWidget
{
public:
  using ConfirmCallback = std::function<void(PlayerCharacter*)>;
  using EscapeCallback = std::function<void()>;

  BattleStatusMenu(const ConfirmCallback& confirmCallback, const EscapeCallback& escapeCallback, int x, int y);

  bool handleInput(sf::Keyboard::Key key) override;
  void draw(sf::RenderTarget& target) override;

  int getWidth() const;
  int getHeight() const;

  /////////////////////////////////////////////////////////////////////////////
  ///
  /// @return  False if no previous actor available.
  ///
  /////////////////////////////////////////////////////////////////////////////
  bool prevActor();

  /////////////////////////////////////////////////////////////////////////////
  ///
  /// @return  False if no next actor available.
  ///
  /////////////////////////////////////////////////////////////////////////////
  bool nextActor();

  PlayerCharacter* getCurrentActor();
  PlayerCharacter* getCurrentSelectedActor();

  void resetActor();

  void setCurrentActorRectHidden(bool hidden) { m_currenActorRectHidden = hidden; }
private:
  void refreshActionMenu();

  int getPartySize() const;
private:
  int m_x, m_y;
  int m_currentActor;
  bool m_currenActorRectHidden;
  int m_index;

  std::vector<PlayerCharacter*> m_party;

  ConfirmCallback m_confirmCallback;
  EscapeCallback m_escapeCallback;
};

#endif /* MENU_BATTLESTATUSMENU_H_ */
