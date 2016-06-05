#include "Cache.h"
#include "Config.h"
#include "Frame.h"

const int partSize = 16;

struct GuiParts
{
  sf::Texture guiTexture;

  sf::Texture topLeft;
  sf::Texture topRight;
  sf::Texture bottomLeft;
  sf::Texture bottomRight;

  sf::Texture topBorder;
  sf::Texture leftBorder;
  sf::Texture rightBorder;
  sf::Texture bottomBorder;
  sf::Texture middle;

  static GuiParts& instance()
  {
    static GuiParts parts;
    return parts;
  }

private:
  GuiParts()
  {
    sf::Image guiImage;
    guiImage.loadFromFile(config::res_path("UI/GUI.png"));

    guiTexture.loadFromImage(guiImage);
    topLeft.loadFromImage(guiImage, sf::IntRect{0, 0, partSize, partSize});
    topRight.loadFromImage(guiImage, sf::IntRect{partSize*2, 0, partSize, partSize});
    bottomLeft.loadFromImage(guiImage, sf::IntRect{0, partSize*2, partSize, partSize});
    bottomRight.loadFromImage(guiImage, sf::IntRect{partSize*2, partSize*2, partSize, partSize});

    topBorder.loadFromImage(guiImage, sf::IntRect{partSize, 0, partSize, partSize});
    rightBorder.loadFromImage(guiImage, sf::IntRect{partSize*2, partSize, partSize, partSize});
    leftBorder.loadFromImage(guiImage, sf::IntRect{0, partSize, partSize, partSize});
    bottomBorder.loadFromImage(guiImage, sf::IntRect{partSize, partSize*2, partSize, partSize});
    middle.loadFromImage(guiImage, sf::IntRect{partSize, partSize, partSize, partSize});

    topBorder.setRepeated(true);
    rightBorder.setRepeated(true);
    leftBorder.setRepeated(true);
    bottomBorder.setRepeated(true);
    middle.setRepeated(true);
  }
};

//void draw_frame(sf::RenderTarget& target, int x, int y, int w, int h)
//{
//  sf::RectangleShape rect;
//  rect.setSize(sf::Vector2f(w - 4, h - 4));
//  rect.setPosition(2 + x, 2 + y);
//  rect.setFillColor(sf::Color::Black);
//  rect.setOutlineColor(sf::Color::White);
//  rect.setOutlineThickness(2.0f);
//  target.draw(rect);
//}

void draw_frame(sf::RenderTarget& target, int x, int y, int w, int h, int thickness)
{
  sf::RectangleShape rect;
  rect.setSize(sf::Vector2f(w - 4, h - 4));
  rect.setPosition(2 + x, 2 + y);
  rect.setFillColor(sf::Color::Black);
  rect.setOutlineColor(sf::Color::White);
  rect.setOutlineThickness(thickness);
  target.draw(rect);
}

void draw_frame(sf::RenderTarget& target, int x, int y, int w, int h)
{
  GuiParts& guiParts = GuiParts::instance();

  sf::Sprite sprite;

  sprite.setTexture(guiParts.middle);
  sprite.setPosition(x + partSize, y + partSize);
  sprite.setTextureRect(sf::IntRect{0, 0, w - partSize*2, h - partSize*2});
  target.draw(sprite);

  sprite.setTexture(guiParts.topBorder);
  sprite.setPosition(x + partSize, y);
  sprite.setTextureRect(sf::IntRect{0, 0, w - partSize*2, partSize});
  target.draw(sprite);

  sprite.setTexture(guiParts.bottomBorder);
  sprite.setPosition(x + partSize, y + h - partSize);
  sprite.setTextureRect(sf::IntRect{0, 0, w - partSize*2, partSize});
  target.draw(sprite);

  sprite.setTexture(guiParts.leftBorder);
  sprite.setPosition(x, y + partSize);
  sprite.setTextureRect(sf::IntRect{0, 0, partSize, h - partSize*2});
  target.draw(sprite);

  sprite.setTexture(guiParts.rightBorder);
  sprite.setPosition(x + w - partSize, y + partSize);
  sprite.setTextureRect(sf::IntRect{0, 0, partSize, h - partSize*2});
  target.draw(sprite);

  sprite.setTextureRect(sf::IntRect{0, 0, partSize, partSize});
  // Top left
  sprite.setTexture(guiParts.topLeft);
  sprite.setPosition(x, y);
  target.draw(sprite);

  // Top right
  sprite.setTexture(guiParts.topRight);
  sprite.setPosition(x + w - partSize, y);
  target.draw(sprite);

  // Bottom right
  sprite.setTexture(guiParts.bottomRight);
  sprite.setPosition(x + w - partSize, y + h - partSize);
  target.draw(sprite);

  // Bottom left
  sprite.setTexture(guiParts.bottomLeft);
  sprite.setPosition(x, y + h - partSize);
  target.draw(sprite);
}
