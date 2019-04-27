#include <iostream>
#include "sky.h"
#include "gfx.h"

  extern Sky* sky;
  extern int scroll_y;
  extern bool gameStarted;
  extern bool introRunning;
  extern int player_knockdown;
  const int scroll_down_speed = 3;

  void print(int start_x, int start_y, const std::string& text, float size=1.0);

extern SpriteSheet* spriteSheetShaman;
void draw();

bool showOutro() {
  static const std::string text[] = {
    "You've made it!",
    "We are saved now!",
    "Thank you!"
  };
  static const int LINE_HEIGHT = 32;
  static const int LETTER_DELAY = 5;
  static const int LINE_DELAY = 30;
  static const int FINAL_DELAY = 100;

  static const int TEXT_POSITION_X = 500;
  static const int TEXT_POSITION_Y = 100;
  static const float FONT_SIZE = 2.0;
  static int delayCount = 0;
  static int letterCount = 0;
  static int lineCount = 0;
  static int nextDelay = LETTER_DELAY;
  static int scrollOut = 0;
  int shamanFrame = 0;
  static bool speech = true;

  draw();
  int y = TEXT_POSITION_Y;
  /*
  for (int i=0; i<lineCount; i++) {
    print(TEXT_POSITION_X, y, text[i], FONT_SIZE);
    y += LINE_HEIGHT;
  }
  */
  if (speech) {
    int line = lineCount;
    if(letterCount == 0 && line > 0) {
      line --;
      print(TEXT_POSITION_X, y, text[line], FONT_SIZE);
    }
    else {
      print(TEXT_POSITION_X, y, text[line].substr(0, letterCount), FONT_SIZE);
    }

    delayCount++;
    if (delayCount >= nextDelay) {
      delayCount = 0;
      if (nextDelay == FINAL_DELAY) {
        speech = false;
      } else {
        letterCount++;
        nextDelay = LETTER_DELAY;
        if (letterCount >= text[lineCount].size()) {
          lineCount++;
          nextDelay = LINE_DELAY;
          if (lineCount >= sizeof(text)/sizeof(text[0])) {
            nextDelay = FINAL_DELAY;
          }
          letterCount = 0;
        }
      }
    }
    if (nextDelay == LETTER_DELAY) {
      shamanFrame = letterCount%2;
    }
  }
  else {
    static int tick = 0;
    tick++;
    shamanFrame = (tick/5)%2 + 2;
    player_knockdown = 1;
  }
  spriteSheetShaman->drawSprite(screen_w/2+16  -32, 11*32 +scroll_y-32-32, shamanFrame);
  static Sky overlay(1.0, 0.0, 0.0, 1.0,
                     1.0, 0.0, 0.0, 1.0,
                     1.0, 0.0, 0.0, 0.0,
                     1.0, 0.0, 0.0, 0.0);
  if (!speech) {
    static int tick = 0;
    tick++;
    overlay.draw(tick/300.0);
  }
  return true;
}

void scrollDown() {
  scroll_y-=scroll_down_speed;
  if (scroll_y <= (-516)+scroll_down_speed) {
    gameStarted = true;
  }
}

bool showIntro() {
  static const std::string text[] = {
    "Dark times have come.",
    "Gods are in great wrath.",
    "New disaster strikes each day.",
    "Thousands will die at this rate.",
    "",
    "Sacrifice needs to be made.",
  };
  static const int LINE_HEIGHT = 32;
  static const int LETTER_DELAY = 5;
  static const int LINE_DELAY = 40;
  static const int FINAL_DELAY = 100;
  static const int TEXT_POSITION_X = 100;
  static const int TEXT_POSITION_Y = 100;
  static const int SCROLL_OUT_MAX = 330;
  static const float FONT_SIZE = 2.0;
  static int delayCount = 0;
  static int letterCount = 0;
  static int lineCount = 0;
  static int nextDelay = LETTER_DELAY;
  static int scrollOut = 0;

  sky->draw(0);
  int y = TEXT_POSITION_Y - scrollOut;
  for (int i=0; i<lineCount; i++) {
    print(TEXT_POSITION_X, y, text[i], FONT_SIZE);
    y += LINE_HEIGHT;
  }
  print(TEXT_POSITION_X, y, text[lineCount].substr(0, letterCount), FONT_SIZE);

  if (scrollOut) {
    scrollOut += scroll_down_speed;
    if (scrollOut >= SCROLL_OUT_MAX) {
      gameStarted = false;
      scroll_y = +200;
      return false;
    }
  }
  delayCount++;
  if (delayCount >= nextDelay) {
    delayCount = 0;
    if (nextDelay == FINAL_DELAY) {
      nextDelay = 999999;
      scrollOut = 1;
    } else {
      letterCount++;
      nextDelay = LETTER_DELAY;
      if (letterCount >= text[lineCount].size()) {
        lineCount++;
        nextDelay = LINE_DELAY;
        if (lineCount >= sizeof(text)/sizeof(text[0])) {
          nextDelay = FINAL_DELAY;
        }
        letterCount = 0;
      }
    }
  }
  return true;
}

