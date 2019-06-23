#pragma once
#include <vector>
#include <memory>
#include <string>
#include "util.h"

class Canvas;

class GuiElement {
  public:
    virtual void draw(Canvas& canvas) = 0;
};

class Gui {
  public:
    Gui(Canvas& canvas) : canvas(canvas) {}
    void draw();
    void addElement(std::unique_ptr<GuiElement>&& element);
  private:
    std::vector<std::unique_ptr<GuiElement>> elements;
    Canvas& canvas;
};

class GuiButton: public GuiElement {
  public:
    GuiButton(const Point& position, const Point& size, const std::string& text);
    void draw(Canvas& canvas) override;
  private:
    Point position;
    Point size;
    std::string text;
};

