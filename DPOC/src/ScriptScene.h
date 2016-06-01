#ifndef SCRIPTSCENE_H_
#define SCRIPTSCENE_H_

#include "Scene.h"
#include "Lua.h"
/*
class ScriptScene : public Scene
{
public:
  ScriptScene(const std::string& scriptFile);
  ~ScriptScene();

  void update();

  void draw(sf::RenderTexture& target);

  void handleEvent(sf::Event& event);

  void preFade(Scene::FadeType fadeType);
  void postFade(Scene::FadeType fadeType);
private:
  lua::LuaEnv m_luaState;
};*/

#endif /* SCRIPTSCENE_H_ */
