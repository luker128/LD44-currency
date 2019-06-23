#pragma once
#include <string>

class Editor;

class EditorState {
  public:
    EditorState(Editor* e, const std::string& name): context(e), name(name) {}
    const std::string& getName() { return name; }
    virtual void draw() {}
    virtual EditorState* mouseLeftDown() { return this; }
    virtual EditorState* mouseLeftUp() { return this; }
    virtual EditorState* mouseRightDown() { return this;}
    virtual EditorState* mouseRightUp() { return this; }
    virtual EditorState* mouseMiddleDown() { return this; }
    virtual EditorState* mouseMiddleUp() { return this; }
    virtual EditorState* mouseMove(int, int) { return this; }
    virtual EditorState* keyPress(int) { return this; }
  protected:
    Editor* context;
    std::string name;
};




class EditorStateIdle: public EditorState {
  public:
    EditorStateIdle(Editor* e): EditorState(e, "Idle") {}
    EditorState* keyPress(int key) override;
    EditorState* mouseLeftDown() override;
};




