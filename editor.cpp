#include <float.h>
#include <GLES3/gl3.h>
#include <cmath>
#include <cstdlib>
#include <string>
#include <set>
#include <map>
#include <tuple>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <memory>
#include "sys/main.h"
#include "gfx/canvas.h"
#include "gfx/gfx.h"
#include "util.h"
#include "level.h"
#include "physics.h"
#include "gui.h"
#include "editor_states.h"


struct NewTriangle {
  std::vector<Point*> points;
  NewTriangle(Point* p = nullptr) {
    if (p != nullptr) {
      points.push_back(p);
    }
  }
};

class Editor {


    Editor(const Editor&) = delete;

    EditorState* state;

  public:

    void changeState(EditorState* newState) {
      if (newState != nullptr && newState != state) {
        delete state;
        state = newState;
      }
    }

    Editor() :
      state(new EditorStateIdle(this)),
      coinImage("data/coin5.png", 64, 64),
      gui(canvas)
    {
      // https://www.deviantart.com/strapaca/art/Brick-wall-seamless-texture-782082949
      textures.emplace_back("data/bricks.png");
      // https://www.deviantart.com/strapaca/art/Diamond-metal-floor-seamless-texture-787512023
      textures.emplace_back("data/steel.png");
      //  https://www.deviantart.com/strapaca/art/Fresh-dark-green-gras-seamless-texture-782082733
      textures.emplace_back("data/grass.png");
      textures.emplace_back("data/roof.png");
      textures.emplace_back("data/gray_wall.png");

      drawTextures = false;
      drawGrid = true;
      shouldSnapToGrid = true;
      loadLevel(level);
///      spawnCoin(200, 100);
      auto button = std::make_unique<GuiButton>(Point(100,100), Point(100, 32), "Hello");

      gui.addElement(std::move(button));
    }

    void keyPressEvent(bool pressed, unsigned char key, unsigned short code) {
      if (pressed == true) {
        //keyPress(key);
        changeState(state->keyPress(key));
      }
      if (key == 229 || key == 225 || key == 16) {
        shiftKey = pressed;
      }
      if (key == 226 || key == 230) {
        altKey = pressed;
      }
      if (key == 82 || key == 38) {
        jumpKey = pressed;
      }
      if (key == 80 || key == 37) {
        leftKey = pressed;
      }
      if (key == 79 || key == 39) {
        rightKey = pressed;
      } 
    }

    void mouseButton(bool pressed, int button) {
      if (pressed == false && button == 1) {
        //mouseUpLeft();
        changeState(state->mouseLeftUp());
      }
      if (pressed == false && button == 3) {
        //mouseUpRight();
        changeState(state->mouseRightUp());
      }
      if (pressed == false && button == 2) {
        mouseUpMiddle();
        changeState(state->mouseMiddleUp());
      }
      if (pressed == true && button == 1) {
        //mouseDownLeft();
        changeState(state->mouseLeftDown());
      }
      if (pressed == true && button == 3) {
        //mouseDownRight();
        changeState(state->mouseRightDown());
      }
      if (pressed == true && button == 2) {
        mouseDownMiddle();
        changeState(state->mouseMiddleDown());
      }
    }

    void mouseWheel(int value) {
      if (value > 0) {
        mouseWheelUp();
      }
      else {
        mouseWheelDown();
      }
    }

    void joyButton(bool pressed, int button) {
      if (pressed == true) {
        jumpButtonPressed();
      } else {
        jumpButtonReleased();
      }
    }


    bool gameLoop() {
      static int last_mouse_x = mouse_x;
      static int last_mouse_y = mouse_y;
      int mickey_x = mouse_x - last_mouse_x;
      int mickey_y = mouse_y - last_mouse_y;
      last_mouse_x = mouse_x;
      last_mouse_y = mouse_y;
      if (mickey_x != 0 || mickey_y != 0) {
        changeState(state->mouseMove(mickey_x, mickey_y));
      }

      mX = (mouse_x - scrollX)  * zoom;
      mY = (mouse_y - scrollY) * zoom;
      cursor = Point(mX, mY);
      static int delay = 0;
//      if ((++delay)%10==0)
      {
        draw();
        update();
      }
      return true;
    }

//  private:


// Input
///////////////////

    void spawnCoin(int x, int y) {
      std::cout << "Spawning coin at " << x << ", " << y << std::endl;
      delete coin;
      coin = new Coin(x,y);
    }

    void mouseWheelUp() {
      zoom *= 1.25;
    }

    void mouseWheelDown() {
      zoom *= 0.75;
    }

    void mouseDownLeft() {/*
      if (newTriangle == nullptr) {
        Point* p = getPointAt(mX, mY);
        Triangle* t = getTriangleAt(cursor);
        if (shiftKey) {
          if (p != nullptr) {
            if (selectedPoints.count(p) == 0) {
              selectedPoints.insert(p);
            }
          }
          else {
            selectingRectangle = true;
            selectingRectangleStart = cursor;
          }
        }
        else {
          if (p == nullptr) {
            selectedPoints.clear();
          }
          else {
            if (pointMerging) {
              Point* first = *selectedPoints.begin();
              mergePoints(first, p);
              pointMerging = false;
            }
            else {
              if (selectedPoints.count(p) == 0 ) {
                selectedPoints.clear();
              }
              selectedPoints.insert(p);
              draggingPoints = true;
              draggingStart = cursor;
            }
          }
          if (t == nullptr) {
            selectedTriangles.clear();
          }
          else {
            if (triangleGrouping) {
              Face* selectedFace = getFaceContaining(**selectedTriangles.begin());
              selectedFace->triangles.push_back(t);
              triangleGrouping = false;
            }
            else {
              if (!altKey) {
                if (selectedTriangles.count(t) == 0 ) {
                  selectedTriangles.clear();
                }
              }
              selectedTriangles.insert(t);
            }
          }
        }
      }
      */
    }


    void mouseUpLeft() {
    }

    void mouseDownRight() {}
    void mouseUpRight() {
    }

    void mouseDownMiddle() {
      scrolling = true;
      scrollStartX = mouse_x;
      scrollStartY = mouse_y;
    }
    void mouseUpMiddle() {
      scrolling = false;
    }

    void keyPress(int key) {
      std::cout << key << std::endl;
      Face* face = getFaceAt(Point(mX, mY));
      if (key == 127 || key == 46) { // delete
        delete coin;
        coin = nullptr;
      }
      if (key == '\r') {
        spawnCoin(mX, mY);
      }

//      if (key == 'l') {
//        loadLevel();
//      }
      if (key == 's') {
        saveLevel(level);
      }
      if (key == 'g') {
        triangleGrouping = true;
      }
      if (key == 'm') {
        pointMerging = true;
      }
    }

    double getJoyValue() {
      int reduced = joy_x / 1024;
      double scaled = reduced / 30.0;
      if (scaled > 1.0) {
        scaled = 1.0;
      }
      if (scaled < -1.0) {
        scaled = -1.0;
      }
      return scaled;
    }

    void jumpButtonPressed() {
      std::cout << "jump" << std::endl;
      if (coin != nullptr) {
        coin->jump = true;
      }
    }

    void jumpButtonReleased() {
      std::cout << "no jump" << std::endl;
      if (coin != nullptr) {
        coin->jump = false;
      }
    }

    typedef std::vector<Image> TextureStorage;

    class TextureSelector {

      public:

        TextureSelector(TextureStorage& textures):
          textures(textures) {}

        void draw(Canvas& canvas) {
          const int columns = 3;
          Point origin = Point(screen_w - columns * offsetHorizontal.x, 0);
          int count = 0;
          for (auto texture: textures) {
            Point pos = origin + offsetHorizontal * (count % columns) + offsetVertical * (count / columns);
            canvas.draw(texture, pos + size/2, size);
            count++;
          }
        }


      private:
        const Point size = Point(64, 64);
        const Point offsetHorizontal = Point(64, 0);
        const Point offsetVertical = Point(0, 64);

        TextureStorage& textures;

    };

// Level editting
///////////////////

    void mergePoints(Point* newPoint, Point* oldPoint) {
      for (Face* face: level.faces) {
        for (Triangle* triangle: face->triangles) {
          for (Edge* edge: triangle->edges) {
            if (edge->a == oldPoint) {
              edge->a = newPoint;
            }
            if (edge->b == oldPoint) {
              edge->b = newPoint;
            }
          }
        }
      }
    }

    Point* getOrCreatePoint(const Point& p) {
      Point* oldPoint = getPointAt(p);
      if (oldPoint != nullptr) {
        return oldPoint;
      }
      return new Point(p);
    }


    double signedArea(Point& a, Point& b, Point& c) {
      return 0.5 * (-1*a.y*b.x + a.x*b.y +a.y*c.x -b.y*c.x -a.x*c.y +b.x*c.y);
    }

    void addTriangleToLevel(const NewTriangle& trianglePoints) {
      Face* newFace = new Face();
      Triangle* triangle = new Triangle();

      {
        Point& a = *trianglePoints.points[0];
        Point& b = *trianglePoints.points[1];
        Point& c = *trianglePoints.points[2];
        double sa = signedArea(a, b, c);
        if (sa < 0) {
          auto tmp = b;
          b = c;
          c = tmp;
        }
        triangle->edges.push_back(new Edge(a, b));
        triangle->edges.push_back(new Edge(b, c));
        triangle->edges.push_back(new Edge(c, a));
      }

      newFace->triangles.push_back(triangle);
      faces.push_back(newFace);
    }
// Polygon math
///////////////////


    Point* getPointAt(const Point& p) {
      for (auto& face: faces) {
        for (auto& triangle: face->triangles) {
          for (auto& edge: triangle->edges) {
            Point* point = edge->a;
            int dx = abs(p.x - point->x);
            int dy = abs(p.y - point->y);
            if (std::max(dx, dy) <= POINT_SIZE) {
              return point;
            }
          }
        }
      }
      return nullptr;
    }

    Point* getPointAtCursor() {
      return getPointAt(cursor);
    }

    bool sideOfLine(double x, double y, Point* a, Point* b) {
      double dy = (b->y - a->y);
      double dx = (b->x - a->x);
      if (dx == 0) {
        if (a->y > b->y) {
          return x > a->x;
        }
        else {
          return x < a->x;
        }
      }
      double direction = dy/dx;
      double position = a->y - direction*a->x;
      bool relation = y > direction*x + position;
      bool result = (relation == (dx < 0));
      if (result == true) {
        return false;
      }
      else {
        return true;
      }
    }

    Face* getFaceContaining(const Triangle& t) {
      for (auto face: faces) {
        for (auto triangle: face->triangles) {
          if (triangle == &t) {
            return face;
          }
        }
      }
      return nullptr;
    }

    bool sideOfEdge(const Point& point, const Edge& edge) {
      return sideOfLine(point.x, point.y, edge.a, edge.b);
    }

    bool isInTriangle(const Point& point, const Triangle& triangle) {
      for (auto edge: triangle.edges) {
        bool side = sideOfEdge(point, *edge);
        if (side == false) {
          return false;
        }
      }
      return true;
    }

    Triangle* getTriangleAt(const Point& p) {
      for (auto face: faces) {
        for (auto triangle: face->triangles) {
          if (isInTriangle(p, *triangle)) {
            return triangle;
          }
        }
      }
      return nullptr;
    }

    Face* getFaceAt(const Point& point) { // TODO: inefficient, rewrite
      Triangle* triangle = getTriangleAt(point);
      if (triangle == nullptr) {
        return nullptr;
      }
      return getFaceContaining(*triangle);
    }

    Point snapToGrid(const Point& p) {
      if (shouldSnapToGrid) {
        return Point(round(p.x/gridSize)*gridSize, round(p.y/gridSize)*gridSize);
      }
      return p;
    }

    void update() {
      if (scrolling) {
        scrollX += (mouse_x - scrollStartX);
        scrollY += (mouse_y - scrollStartY);
        scrollStartX = mouse_x;
        scrollStartY = mouse_y;
      }
      if (draggingPoints) {
        Point mousePosition(mX, mY);
        mousePosition = snapToGrid(mousePosition);
        if (snapToGrid(mousePosition - draggingStart) != Point(0,0)) {
          for (Point* point: selectedPoints) {
            *point = snapToGrid(*point + (mousePosition - draggingStart));
          }
          draggingStart = mousePosition;
        }
      }
      if (coin != nullptr) {
        updateCoin(*coin, polygons, leftKey, rightKey, jumpKey, getJoyValue());
      }
    }


// Drawing
///////////////////

    void drawPoint(const Point& point) {
      static const int SIZE = POINT_SIZE;
      canvas.drawRectangle(point.x * zoom - SIZE, point.y * zoom - SIZE,
                         point.x * zoom + SIZE, point.y * zoom + SIZE);
    }

    void drawLine(const Point& a, const Point& b) {
      canvas.drawLine(a.x * zoom, a.y * zoom,
                    b.x * zoom, b.y * zoom);
    }

    void drawFaceTextured(const Face& face) {
      std::vector<float> positions;
      for (auto triangle: face.triangles) {
        for (auto edge: triangle->edges) {
          Point* point = edge->a;
          positions.push_back(point->x * zoom);
          positions.push_back(point->y * zoom);
        }
      }
      glBindTexture(GL_TEXTURE_2D, textures[face.textureId].getTexture());
      canvas.drawConvexPolygon(positions, face.textureScale / zoom, face.textureX * zoom, face.textureY * zoom);
    }

    void drawFaceWireframe(const Face& face) {
      std::set<Point*> pointsToDraw;
      for (auto triangle: face.triangles) {
        if (selectedTriangles.count(triangle) > 0) {
          canvas.setColor(0,0,1,1);
        }
        else {
          canvas.setColor(1,1,1,1);
        }
        for (auto edge: triangle->edges) {
          drawLine(*edge->a, *edge->b);
          pointsToDraw.insert(edge->a);
          pointsToDraw.insert(edge->b);
          Point v = edge->getVector();
          Point n = perpendicular(v) / len(v);
          Point mid = *edge->a + (v * 0.5);
          drawLine(mid, mid + n*10);
        }
      }
      for (auto point: pointsToDraw) {
        if (selectedPoints.count(point) > 0) {
          canvas.setColor(0,0,1,1);
        }
        else {
          canvas.setColor(1,1,1,1);
        }
        drawPoint(*point);
        canvas.setColor(1,1,1,1);
      }
    }

    void drawFace(const Face& face) {
      if (drawTextures) {
        drawFaceTextured(face);
      }
      drawFaceWireframe(face);
    }

    void drawNewTriangle(NewTriangle triangle) {
      if (triangle.points.size() > 0) {
        canvas.setColor(1,1,0,1);
        for (int i=0; i<triangle.points.size()-1; i++) {
          drawLine(*triangle.points[i], *triangle.points[i+1]);
        }
        drawLine(*triangle.points[triangle.points.size()-1], Point(mX, mY));
      }
    }

    std::string getModeName() {
      return state->getName();
    }

    void printStatus() {
      canvas.setColor(0,0,0, 1);
      Triangle* triangle = getTriangleAt(Point(mX, mY));
      Face* face = nullptr;
      if (triangle != nullptr) {
        face = getFaceContaining(*triangle);
      }
      Point* point = getPointAt(cursor);
      std::ostringstream os;
      os << "Mode: " << getModeName() << "\n";
      os << "Face: " << face << "\n";
      os << "Triangle: " << triangle << "\n";
      if (face) {
        os << "  textureId: " << face->textureId << "\n";
        os << "  textureScale: " << face->textureScale << "\n";
        os << "  textureX/Y: " << Point(face->textureX, face->textureY) << "\n";
      }
      os << "Point:" << point << "\n";
      if (point) {
        os << "  " << *point << "\n";
      }
      canvas.print({32,32}, os.str());
      os.str("");
      os.clear();
      os << "Selected points: " << selectedPoints.size() << "\n";
      os << "Selected triangles: " << selectedTriangles.size() << "\n";
      canvas.print({32,screen_h-32.0}, os.str());
    }

    void clear() {
      glClear(GL_COLOR_BUFFER_BIT);
      if (drawGrid) {
        canvas.setColor((101/255.0)*0.9, (184/255.0)*0.9, (227/225.0)*0.9, 1.0);
        canvas.setColor(0,0,0, 0.06);
        for (int y=0; y<screen_h/gridSize; y++) {
          canvas.drawLine(0,gridSize*y, screen_w, gridSize*y);
        }
        for (int x=0; x<screen_w/gridSize; x++) {
          canvas.drawLine(gridSize*x, 0, gridSize*x, screen_h);
        }
      }
    }

    void drawCoin() {
      canvas.setScroll({(screen_w/2)-coin->position.x,
                     (screen_h/2)-coin->position.y});
      if (coin->velocity.x != 0) {
        coin->rotation += coin->velocity.x / COIN_SIZE;
      }
      //coinImage.drawSpriteRotated(coin->position.x+scrollX, coin->position.y+scrollY, 0, coin->rotation);
      coinImage.drawSpriteRotated(screen_w / 2, screen_h /2, 0, coin->rotation);
      if (!drawTextures) {
        canvas.drawLine(coin->position.x, coin->position.y,
                      coin->position.x + coin->velocity.x * 30,
                      coin->position.y + coin->velocity.y * 30
            );
      }
    }

    void draw() {
      clear();
      if (coin != nullptr) {
        drawCoin();
      }
      else {
        canvas.setScroll({scrollX, scrollY});
    //    canvas.setScale(zoom);
      }
      canvas.setColor(1,1,1,1);
      for (Face* face: faces) {
        drawFace(*face);
      }
      printStatus();
      state->draw();
      gui.draw();
      textureSelector.draw(canvas);
    }


    Level level;
    std::vector<Face*>& faces = level.faces;

    Coin* coin = nullptr;
    bool drawTextures = false;
    Canvas canvas;
    SpriteSheet coinImage;
    std::vector<Polygon*> polygons;

    std::set<Point*> selectedPoints;
    std::set<Triangle*> selectedTriangles;
    bool triangleGrouping = false;
    bool pointMerging = false;
    bool draggingPoints = false;
    Point draggingStart = Point(0,0);
    bool shiftKey = false;
    bool altKey = false;
    Point selectingRectangleStart = Point(0,0);
    bool selectingRectangle = false;
    bool drawGrid = false;
    bool shouldSnapToGrid = false;
    float gridSize = 10.0;

    TextureStorage textures;
    TextureSelector textureSelector = TextureSelector(textures);
    double scrollX = 0;
    double scrollY = 0;
    float zoom = 1.0;
    bool scrolling = false;
    int scrollStartX;
    int scrollStartY;
    int mX;
    int mY;
    Point cursor = Point(mX, mY);
    bool leftKey = false;
    bool rightKey = false;
    bool jumpKey = false;

    Gui gui;
};


#include "states.h"


Editor* editor = nullptr;


void key_press(bool pressed, unsigned char key, unsigned short code) {
  editor->keyPressEvent(pressed, key, code);
}

void mouse_button(bool pressed, int button, int x, int y ) {
  editor->mouseButton(pressed, button);
}

void mouse_wheel(int value) {
  editor->mouseWheel(value);
}

void joy_button(bool pressed, int button) {
  editor->joyButton(pressed, button);
}

bool gameLoop() {
  return editor->gameLoop();
}

void gameInit() {
  const int w = 1600;
  const int h  = (w/16.0)*9.0;
  createWindow(w, h, "Currency - Editor");
  glViewport(0, 0, screen_w, screen_h);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(101/255.0 * 0.5,
               184/255.0 * 0.5,
               227/225.0 * 0.5, 1.0);
  editor = new Editor();
}

void gameCleanup() {
  delete editor;
}

