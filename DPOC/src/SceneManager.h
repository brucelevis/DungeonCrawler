#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <vector>
#include <map>
#include <string>

#include <SFML/Graphics.hpp>

#include "Scene.h"

class Picture;

class SceneManager
{
public:
  static SceneManager& instance();
  void create();

  void run();

  void addScene(Scene* scene) { m_scenes.push_back(scene); }

  void shakeScreen(int duration, int shakeStrengthX, int shakeStrengthY);
  bool isShaking() const { return m_shakeCounter > 0; }

  void flashScreen(int duration, sf::Color color);

  void close();

  void fadeIn(int duration);
  void fadeOut(int duration);

  void showPicture(const std::string& pictureName, float x, float y);
  void hidePicture(const std::string& pictureName);
private:
  SceneManager();

  void cleanUp();

  void draw();

  void pollEvents();
  void processFade();
private:
  std::vector< Scene* > m_scenes;

  sf::RenderWindow m_window;
  sf::RenderTexture m_targetTexture;

  int m_shakeCounter;
  int m_shakeStrengthX;
  int m_shakeStrengthY;

  Scene::FadeType m_fade;
  int m_fadeCounter;
  int m_fadeDuration;

  std::map<std::string, Picture*> m_pictures;

  int m_flashDuration;
  int m_flashCounter;
  sf::Color m_flashColor;
};

#endif
