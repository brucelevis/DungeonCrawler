#include "logger.h"
#include "LuaBindings.h"
#include "ScriptScene.h"

ScriptScene::ScriptScene(const std::string& scriptFile)
{
  register_lua_bindings(m_luaState);

  if (!m_luaState.executeFile(scriptFile))
  {
    TRACE("Failed to load script: %s", scriptFile.c_str());
  }
  else
  {
    m_luaState.call_function("constructor", this);
  }
}

ScriptScene::~ScriptScene()
{
  m_luaState.call_function("destructor", this);
}

void ScriptScene::update()
{
  m_luaState.call_function("update", this);
}

void ScriptScene::draw(sf::RenderTarget& target)
{
  m_luaState.call_function("draw", this, &target);
}

void ScriptScene::handleEvent(sf::Event& event)
{
  m_luaState.call_function("handleEvent", this, &event);
}

void ScriptScene::preFade(Scene::FadeType fadeType)
{
  m_luaState.call_function("preFade", this, static_cast<int>(fadeType));
}

void ScriptScene::postFade(Scene::FadeType fadeType)
{
  m_luaState.call_function("postFade", this, static_cast<int>(fadeType));
}
