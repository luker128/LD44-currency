#include "editor_states.h"

class EditorStateSelecting: public EditorState {
  public:
    EditorStateSelecting(Editor* e): EditorState(e, "Selecting"), selectionOrigin(mouse_x, mouse_y) {}
    EditorState* mouseMove(int dx, int dy) override {
      selectionSize += Point(dx, dy);
      return this;
    }

    std::set<Point*> findPoints() {
      std::set<Point*> result;
      Point selectionEnd = selectionOrigin + selectionSize;
      float rectx0 = std::min(selectionOrigin.x, selectionEnd.x);
      float recty0 = std::min(selectionOrigin.y, selectionEnd.y);
      float rectx1 = std::max(selectionOrigin.x, selectionEnd.x);
      float recty1 = std::max(selectionOrigin.y, selectionEnd.y);
      for (Face* face: context->level.faces) {
        for (Triangle* triangle: face->triangles) {
          for (Edge* edge: triangle->edges) {
            Point* point = edge->a;
            if (point->x > rectx0 && point->x < rectx1 &&
                point->y > recty0 && point->y < recty1) {
              result.insert(point);
            }
          }
        }
      }
      return result;
    }
    EditorState* mouseLeftUp() override {
      if (context->shiftKey == false) {
        context->selectedPoints.clear();
      }
      auto pointsInArea = findPoints();
      for (auto point: pointsInArea) {
        context->selectedPoints.insert(point);
      }
      return new EditorStateIdle(context);
    }
    void draw() override {
      context->canvas.setColor(1,1,1);
      context->canvas.drawLine(selectionOrigin.x, selectionOrigin.y, selectionOrigin.x + selectionSize.x, selectionOrigin.y);
      context->canvas.drawLine(selectionOrigin.x, selectionOrigin.y, selectionOrigin.x, selectionOrigin.y + selectionSize.y);
      context->canvas.drawLine(selectionOrigin.x + selectionSize.x, selectionOrigin.y + selectionSize.y, selectionOrigin.x, selectionOrigin.y + selectionSize.y);
      context->canvas.drawLine(selectionOrigin.x + selectionSize.x, selectionOrigin.y + selectionSize.y, selectionOrigin.x + selectionSize.x, selectionOrigin.y);
    }
  private:
    Point selectionOrigin;
    Point selectionSize = {0,0};
};

class EditorStateMovingPoint: public EditorState {
  public:
    EditorStateMovingPoint(Editor* e): EditorState(e, "Moving point") {
      Point* point = context->getPointAtCursor();
      if (context->selectedPoints.count(point) == 0) {
        if (context->shiftKey == false) {
          context->selectedPoints.clear();
        }
        context->selectedPoints.insert(point);
      }
      else {
        if (context->shiftKey == true) {
          context->selectedPoints.erase(point);
        }
      }
    }
    EditorState* mouseLeftUp() override {
      return new EditorStateIdle(context);
    }
    virtual EditorState* mouseMove(int dx, int dy) override {
      Point mickey(dx, dy);
      for (auto point: context->selectedPoints) {
        *point += mickey;
      }
      return this;
    }
};

class EditorStateNewTriangle: public EditorState {
  public:
    EditorStateNewTriangle(Editor* e): EditorState(e, "Creating triangle") {}
    void draw() override {
      context->drawNewTriangle(newTriangle);
    }
    EditorState* mouseLeftUp() override {
      Point* point = context->getOrCreatePoint(context->cursor);
      newTriangle.points.push_back(point);
      if (newTriangle.points.size() == 3) {
        context->addTriangleToLevel(newTriangle);
        return new EditorStateIdle(context);
      }
      return this;
    }
    EditorState* mouseRightUp() override {
      newTriangle.points.pop_back();
      return this;
    }
    NewTriangle newTriangle;
};

class EditorStateMergePoints: public EditorState {
  public:
    EditorStateMergePoints(Editor* e): EditorState(e, "Merging points") {}
    EditorState* mouseLeftUp() override {
      Point* point = context->getPointAtCursor();
      if (point != nullptr) {
        return new EditorStateIdle(context);
      }
      return this;
    }
};

EditorState* EditorStateIdle::keyPress(int key) {
  Face* face = context->getFaceAt(context->cursor);
  if (key == ' ') {
    return new EditorStateNewTriangle(context);
  }
  else if (key == 'm') {
    return new EditorStateMergePoints(context);
  }
  else if (key == 'f') {
    context->drawTextures = !context->drawTextures;
  }
  else if (key == '[') {
    face->textureScale *= 0.5;
  }
  else if (key == ']') {
    face->textureScale *= 2.0;
  }
  else if (key == 'j') {
    face->textureY += 1;
  }
  else if (key == 'k') {
    face->textureY -= 1;
  }
  else if (key == 'h') {
    face->textureX -= 1;
  }
  else if (key == 'l') {
    face->textureX += 1;
  }
  else if (key == 75 || key == 33) { // pg up
    face->textureId++;
    if (face->textureId == context->textures.size()) {
      face->textureId = 0;
    }
  }
  else if (key == 78 || key == 34) { // pg dn
    face->textureId--;
    if (face->textureId == -1) {
      face->textureId = context->textures.size()-1;
    }
  }
  return this;
}

EditorState* EditorStateIdle::mouseLeftDown() {
  auto point = context->getPointAtCursor();
  if (point) {
    return new EditorStateMovingPoint(context);
  } else {
    return new EditorStateSelecting(context);
  }
}





