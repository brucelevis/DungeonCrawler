#include "logger.h"
#include "ScriptMenu.h"

ScriptMenu::ScriptMenu(const std::string& scriptFile)
{
  m_luaState.register_function("_super_draw", &ScriptMenu::baseDraw);
  m_luaState.register_function("_super_handle_escape", &ScriptMenu::baseHandleEscape);
  m_luaState.register_function("_super_move_arrow", &ScriptMenu::baseMoveArrow);
  m_luaState.register_function("_super_get_width", &ScriptMenu::baseGetWidth);
  m_luaState.register_function("_super_get_height", &ScriptMenu::baseGetHeight);
  m_luaState.register_function("_protected_set_max_visible", [this](int maxVisible) { setMaxVisible(maxVisible); });
  m_luaState.register_function("_protected_draw_select_arrow", [this](sf::RenderTarget* target, int x, int y) { drawSelectArrow(*target, x, y); });

  if (!m_luaState.executeFile(scriptFile))
  {
    TRACE("Failed to load script: %s [%s]", scriptFile.c_str(), m_luaState.getError().c_str());
  }
  else
  {
    m_luaState.call_function("constructor", this);
  }
}

ScriptMenu::~ScriptMenu()
{
  m_luaState.call_function("destructor", this);
}

void ScriptMenu::handleConfirm()
{
  m_luaState.call_function("handle_confirm", this);
}

void ScriptMenu::handleEscape()
{
  m_luaState.call_function("handle_escape", this);
}

void ScriptMenu::moveArrow(Direction dir)
{
  m_luaState.call_function("move_arrow", this, static_cast<int>(dir));
}

void ScriptMenu::draw(sf::RenderTarget& target, int x, int y)
{
  m_luaState.call_function("draw", this, &target, x, y);
}

int ScriptMenu::getWidth() const
{
  return m_luaState.call_function_result<int>("get_width", this);
}

int ScriptMenu::getHeight() const
{
  return m_luaState.call_function_result<int>("get_height", this);
}

void ScriptMenu::baseDraw(sf::RenderTarget* target, int x, int y)
{
  Menu::draw(*target, x, y);
}

void ScriptMenu::baseHandleEscape()
{
  Menu::handleEscape();
}

void ScriptMenu::baseMoveArrow(int dir)
{
  Menu::moveArrow(static_cast<Direction>(dir));
}

int ScriptMenu::baseGetWidth() const
{
  return Menu::getWidth();
}

int ScriptMenu::baseGetHeight() const
{
  return Menu::getHeight();
}

void ScriptMenu::invalidate()
{
  m_luaState.call_function("invalidate", this);
}
