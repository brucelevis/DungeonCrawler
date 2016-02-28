#ifndef SCRIPTSCENE_H_
#define SCRIPTSCENE_H_

#include "Scriptable.h"
#include "Scene.h"
#include "Lua.h"

class ScriptScene : public Scene, public Scriptable
{
public:
  ScriptScene(const std::string& scriptFile);
  ~ScriptScene();

  void update();

  void draw(sf::RenderTarget& target);

  void handleEvent(sf::Event& event);

  void preFade(Scene::FadeType fadeType);
  void postFade(Scene::FadeType fadeType);
};

#endif /* SCRIPTSCENE_H_ */
