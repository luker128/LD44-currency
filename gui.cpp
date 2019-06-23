#include "gui.h"
#include "gfx/canvas.h"

void Gui::draw() {
  for (auto& element: elements) {
    element->draw(canvas);
  }
}

void Gui::addElement(std::unique_ptr<GuiElement>&& element) {
  elements.emplace_back(std::move(element));
}

GuiButton::GuiButton(const Point& position, const Point& size, const std::string& text) :
  position(position),
  size(size),
  text(text) {}

void GuiButton::draw(Canvas& canvas) {
  canvas.setColor(0.4, 0.4, 0.4);
  canvas.drawRectangle(position.x, position.y,
      position.x + size.x, position.y + size.y);
  canvas.print(position + Point{32,24}, text);
}

