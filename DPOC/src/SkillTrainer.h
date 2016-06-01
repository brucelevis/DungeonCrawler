#ifndef SKILLTRAINER_H_
#define SKILLTRAINER_H_

#include <vector>
#include <string>

#include "Menu.h"
#include "Scene.h"

class PlayerCharacter;
class TrainMenu;

class SkillTrainer : public Scene
{
public:
  SkillTrainer(const std::vector<std::string>& skills);
  ~SkillTrainer();

  void update();
  void draw(sf::RenderTexture& target);
  void handleEvent(sf::Event& event);
private:
  void handleKeyPress(sf::Keyboard::Key key);
  void escape();
  PlayerCharacter* getCurrentCharacter();
private:
  std::vector<std::string> m_skills;
  ChoiceMenu m_choiceMenu;
  TrainMenu* m_trainMenu;
};

#endif /* SKILLTRAINER_H_ */
