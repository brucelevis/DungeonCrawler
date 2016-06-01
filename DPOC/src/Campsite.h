#ifndef CAMPSITE_H_
#define CAMPSITE_H_

#include <SFML/Audio.hpp>
#include <string>

#include "Menu.h"
#include "Scene.h"

class Campsite : public Scene
{
  enum State
  {
    STATE_STARTING,
    STATE_START_MESSAGE,
    STATE_MESSAGE_ONGOING,
    STATE_CHOOSE,
    STATE_LEAVE,
    STATE_REST
  };
public:
  Campsite();
  ~Campsite();

  void update();

  void draw(sf::RenderTexture& target);

  void handleEvent(sf::Event& event);

  void preFade(Scene::FadeType fadeType);
  void postFade(Scene::FadeType fadeType);
private:
  void rest();
  void leave();
  int requiredFood() const;

  void startMessage(const std::string& message);
private:
  State m_state;
  ChoiceMenu m_menu;
  int m_requiredFood;

  sf::Music m_restMusic;
  std::string m_currentMessage;

  sf::Texture* m_background;

  bool m_draw;
};

#endif /* CAMPSITE_H_ */
