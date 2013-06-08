//#include <SFML/Graphics.hpp>

#include "logger.h"

#include "draw_text.h"
#include "Editor.h"

int main()
{
//  sf::RenderWindow window;
//  window.create(sf::VideoMode(640, 480), "Window!");
//
//  while (window.isOpen())
//  {
//    sf::Event event;
//
//    while (window.pollEvent(event))
//    {
//      switch (event.type)
//      {
//      case sf::Event::Closed:
//        window.close();
//        break;
//      default:
//        break;
//      }
//    }
//
//    window.clear();
//    window.display();
//  }

  START_LOG;

  init_text_drawing();

  Editor editor;
  editor.run();

  return 0;
}
