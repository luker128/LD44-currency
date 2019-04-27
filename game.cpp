#include <GLES3/gl3.h>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "main.h"
#include "gfx.h"
#include "sky.h"
#include "level.h"



  Image* tileset;
  Image* tilesetSnake;
  Image* tilesetFireball;
  Image* tilesetFont;
  Image* tilesetShaman;
  SpriteSheet* spriteSheet;
  SpriteSheet* spriteSheetSnake;
  SpriteSheet* spriteSheetFireball;
  SpriteSheet* spriteSheetFont;
  SpriteSheet* spriteSheetShaman;
  Tilemap* tilemapFloor;
  Sky* sky;

  const int GRID_SIZE = 32;
  const int MAX_PLAYER_MOVING = 5;

  const int MAX_NEXT_GRAVITY = 5;
  int nextGravity = MAX_NEXT_GRAVITY;
  const int FIREBALLS_START_LEVEL = 740;
  const int ROCKS_START_LEVEL = 835;
  const int FINISH_LEVEL = 352;

  int scroll_y = -500;

  int player_x = 900;
  int player_y = 1056-32;
  float player_vx = 0;
  float player_vy = 0;
  int player_moving = 0;
  bool player_climbing = false;
  int player_jumping = 0;
  int player_knockback = 0;
  int player_knockdown = 0;
  bool player_knockback_flipped = false;
  bool player_riding = false;
  const int MAX_JUMP = 18;

  bool key_left = false;
  bool key_right = false;
  bool key_up = false;
  bool key_down = false;
  bool key_jump = false;

  int quake = 0;
  int rumble_x = 0;
  int rumble_y = 0;

  bool gameStarted = false;
  bool outroRunning = false;
  bool introRunning = true;

  bool fireballs = false;
  bool rocks = false;


void print(int start_x, int start_y, const std::string& text, float size=1.0) {
  int x = start_x - 8;
  int y = start_y - 8;
  const float kern = 0.8;
  for (char c: text) {
    spriteSheetFont->drawSpriteScaled(x, y, c, size);
    x += spriteSheetFont->getFrameWidth() * kern * size;
  }
}


#include "mobs.h"


std::vector<Mob*> mobs;


void showStatus() {
  if (player_climbing) {
    spriteSheet->drawSprite(0,0,  11);
  }
  int tile = tileAt(player_x/GRID_SIZE, (player_y+1)/GRID_SIZE);
  bool on_ground = isTileSolid(tile);
  if (on_ground) {
    spriteSheet->drawSprite(32,0, 12);
  }
  if (player_jumping) {
    spriteSheet->drawSprite(64,0, 13);
  }
  if (player_knockdown) {
    spriteSheet->drawSprite(64+32,0, 14);
  }
  std::ostringstream os;
  os << "level: " << player_y;
  print(0,40, os.str());
  os.str("");
  os << "fireballs: " << fireballs;
  print(0,56, os.str());
}

void draw() {
  if (gameStarted) {
    sky->draw((516+scroll_y)/516.0);
  } else {
    sky->draw(0);
  }
  if (quake > 0) {
    rumble_x = rand()%20-10;
    rumble_y = rand()%20-10;
    quake--;
    if (quake == 0) {
      rumble_x = 0;
      rumble_y = 0;
    }
  }
  tilemapFloor->drawTilemap(rumble_x,rumble_y+scroll_y);

  if (player_knockdown) {
    static float frame = 12;
    frame+= 0.1;
    if (frame >= 14) {
      frame = 12;
    }
    spriteSheetShaman->drawSprite(player_x-16+rumble_x-16, player_y-32+scroll_y+rumble_y-32, (int)frame);
  }
  else {
    if (player_vx == 0) {
      spriteSheetShaman->drawSprite(player_x-16+rumble_x-16, player_y-32+scroll_y+rumble_y-32,  4);
    } else {
      static float frame = 6;
      if (player_vx < 0) {
        spriteSheetShaman->drawSprite(player_x-16+rumble_x-16, player_y-32+scroll_y+rumble_y-32, (int)frame);
      } else {
        spriteSheetShaman->drawSpriteFlipped(player_x+16+rumble_x+16, player_y-32+scroll_y+rumble_y-32, (int)frame);
      }
      frame+= 0.2;
      if (frame >= 12) {
        frame = 6;
      }
    }
  }
//  print(player_x, player_y+scroll_y, "x");
  if (!outroRunning) {
    for (auto mob : mobs) {
      if (!mob->isDead()) {
        mob->draw();
      }
    }
  }
//  showStatus();
}

void movePlayer(int new_x, int new_y) {
  player_x = new_x;
  player_y = new_y;
}

void updatePlayerPosition() {
  const int vx = 4;
  const int vy = 4;
  int dx = 0;
  int dy = 0;
  float ax = 0;
  float ay = 0;
  const float ACCEL_X = 0.4;
  const float DECEL_X = 1.5;
  const float AIR_DECEL_X = 0.1;
  const float JUMP_ACCEL = 4;
  const float GRAVITY = 0.3;
  const float player_vmaxx = 4;
  const float player_vmaxy = 6;

  int tile = tileAt(player_x/GRID_SIZE, (player_y+1)/GRID_SIZE);
  bool on_ground = isTileSolid(tile);

  if (!player_climbing && !quake) {
    // moving left/right
    if (!player_knockback) {
      if (key_left) {
        ax = -ACCEL_X;
      }
      if (key_right) {
        ax = +ACCEL_X;
      }
    }
    // Deceleration
    if (!key_left && !key_right) {
      float decel = DECEL_X;
      if (!on_ground) {
        decel = AIR_DECEL_X;
      }
      if (player_vx > 0) {
        if (player_vx < decel) {
          player_vx = 0;
        } else {
          ax = -decel;
        }
      }
      if (player_vx < 0) {
        if (player_vx > -decel) {
          player_vx = 0;
        } else {
         ax = +decel;
        }
      }
    }
  }

  if (key_up) {
    int tile = tileAt(player_x/GRID_SIZE, (player_y-vy)/GRID_SIZE);
    if (isTileLadder(tile)) {
      player_vy = -vy;
      player_climbing = true;
    } else {
      if (player_climbing) {
        for (int i=player_y;; i--) {
          int tile = tileAt(player_x/GRID_SIZE, i/GRID_SIZE);
          if (!isTileLadder(tile)) {
            player_y = i;
            break;
          }
        }
        player_vy = 0;
        player_climbing = false;
      }
    }
  }
  if (key_down) {
    int tile = tileAt(player_x/GRID_SIZE, (player_y+vy)/GRID_SIZE);
    if (isTileLadder(tile)) {
      player_vy = +vy;
      player_climbing = true;
    } else {
      if (player_climbing) {
        for (int i=player_y;; i++) {
          int tile = tileAt(player_x/GRID_SIZE, (i)/GRID_SIZE);
          if (!isTileLadder(tile)) {
            player_y = i;
            break;
          }
        }
        player_vy = 0;
        player_climbing = false;
      }
    }
  }
  if (player_climbing) {
    player_jumping = false;
    player_vx = 0;
    if (!key_down && !key_up) {
      player_vy = 0;
    }
  }


  if (!player_jumping) {
    // jumping 
    int tile = tileAt(player_x/GRID_SIZE, (player_y+1)/GRID_SIZE);
    bool on_ground = isTileSolid(tile);
    if (on_ground || player_riding) {
      if (key_jump) {
        player_jumping = MAX_JUMP;
        player_vy = -JUMP_ACCEL;
      }
    }
  }

  // gravity
  if (!player_climbing && !player_riding) {
    if (!on_ground || player_jumping || player_knockdown) {
      ay = GRAVITY;
    }
  }

  if (player_knockdown) {
    player_knockdown--;
  }

  player_vx += ax;
  player_vy += ay;
  if (player_vx >= player_vmaxx) {
    player_vx = player_vmaxx;
  }
  if (player_vx <= -player_vmaxx) {
    player_vx = -player_vmaxx;
  }
  if (player_vy >= player_vmaxy) {
    player_vy = player_vmaxy;
  }
  if (player_vy <= -player_vmaxy) {
    player_vy = -player_vmaxy;
  }
  // landing on ground
  if (!player_climbing && player_vy > 0 && !player_knockdown && !player_riding) {
    int nextTile = tileAt(player_x/GRID_SIZE, (player_y+player_vy)/GRID_SIZE);
    int currentTile =tileAt(player_x/GRID_SIZE, (player_y)/GRID_SIZE);
    if (isTileSolid(nextTile) && !isTileLadder(nextTile)) {
      for (int i=player_y+player_vy;;i--) {
        int previousTile = tileAt(player_x/GRID_SIZE, i/GRID_SIZE);
        if (!isTileSolid(previousTile) || isTileLadder(previousTile)) {
          player_y = i+1;
          player_vy = 0;
          player_jumping = false;
          player_knockback = false; // ?
          break;
        }
      }
    }
    else if ((isTileLadder(nextTile) && !isTileSolid(currentTile))) {
      for (int i=player_y+player_vy;;i--) {
        int previousTile = tileAt(player_x/GRID_SIZE, i/GRID_SIZE);
        if (!isTileSolid(previousTile)) {
          player_y = i+1;
          player_vy = 0;
          player_jumping = false;
          break;
          player_knockback = false; // ?
        }
      }
    }
  }
  movePlayer(player_x + player_vx, player_y + player_vy);
}

void spawnFireBall() {
  const int range = 300;
  int x = (rand() % range - range/2) + screen_w/2;
//  int y = player_y - screen_h/2 - 64;
  int y = player_y - (player_y+scroll_y) - 64;
  mobs.push_back(new Fireball(x, y));
}

void maybeSpawnFireball() {
  static int timeSinceLast = 0;
  int r = rand()%25000;
  timeSinceLast++;
  if (r < timeSinceLast*3) {
    spawnFireBall();
    timeSinceLast = 0;
  }
}

Rock* lastRock = nullptr;

void spawnRock(int y=830, int x=30) {
  mobs.push_back(new Rock(x,y));
  lastRock = static_cast<Rock*>(mobs.back());
}

void maybeSpawnRock(int y = -1) {
  if (lastRock && lastRock->isDead()) {
    spawnRock(lastRock->getLevel(), lastRock->getOrigin());
  }
}

void restartGame() {
  player_x = 900;
  player_y = 1056-32;
  player_vx = 0;
  player_vy = 0;
  player_moving = 0;
  player_climbing = false;
  player_jumping = 0;
  player_knockback = 0;
  player_knockdown = 0;
  scroll_y = -516;
}

void update() {
  if (player_y < FINISH_LEVEL) {
    outroRunning = true;
//    gameStarted = false;
  }
  if (player_y > 1050) {
    restartGame();
  }
  if (rocks) {
    maybeSpawnRock();
  } else {
    if (player_y <= ROCKS_START_LEVEL) {
      spawnRock();
      quake = 50;
      player_vx=0;
      player_vy=0;
      setLevel(1);
      mobs.push_back(new Shaman(screen_w/2+16, 11*GRID_SIZE));
      rocks = true;
    }
  }

  if (fireballs) {
    maybeSpawnFireball();
  } else {
    if (player_y <= FIREBALLS_START_LEVEL) {
      spawnRock(735, 70);
      quake = 70;
      player_vx=0;
      player_vy=0;
//      setLevel(1);
      fireballs = true;
    }
  }

  player_riding = false;
  for (auto mob : mobs) {
    if (!mob->isDead()) {
      mob->update();
      mob->checkCollision(player_x, player_y);
    }
  }
  updatePlayerPosition();
  int new_scroll_y = -516 + (800 - player_y);
  if (new_scroll_y > scroll_y) {
    scroll_y = new_scroll_y;
  }
  if (player_y + scroll_y > screen_h) { 
    restartGame();
  }
}

void key_press(bool pressed, unsigned char key, unsigned short code) {
  if (key == 13) { // enter
    spawnRock();
  }
  if (key == 32) { // space
    key_jump = pressed;
  }
  if (key == 79 || code == 39) {
    key_right = pressed;
  } else if (key == 80 || code == 37) {
    key_left = pressed;
  } else if (key == 81 || code == 40) {
    key_down = pressed;
  } else if (key == 82 || code == 38) {
    key_up = pressed;
  }
}
void mouse_button(bool pressed, int button, int x, int y ) {
}

void scrollDown();
bool showIntro();
bool showOutro();

bool gameLoop() {
  if (introRunning) {
    introRunning = showIntro();
    return true;
  }
  if (outroRunning) {
    return showOutro();
  }
  if (gameStarted) {
    update();
  } else {
    scrollDown();
  }
  draw();
  return true;
}

void gameInit() {
  createWindow(960, 540, "Sacrifice");
//  createWindow(960, 1080, "Sacrifice");
  glViewport(0, 0, screen_w, screen_h);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  sky = new Sky();
  tileset = new Image("data/tiles.png");
  tilesetSnake = new Image("data/snake.png");
  tilesetFireball = new Image("data/fireball.png");
  tilesetFont = new Image("data/font.png");
  tilesetShaman = new Image("data/shaman.png");
  spriteSheet = new SpriteSheet(*tileset, GRID_SIZE, GRID_SIZE);
  spriteSheetSnake = new SpriteSheet(*tilesetSnake, GRID_SIZE*2, GRID_SIZE*2);
  spriteSheetFireball= new SpriteSheet(*tilesetFireball, GRID_SIZE*2, GRID_SIZE*2);
  spriteSheetFont = new SpriteSheet(*tilesetFont, 16, 16);
  spriteSheetShaman = new SpriteSheet(*tilesetShaman, 64, 64);
  tilemapFloor = new Tilemap(*tileset, MAP_W, MAP_H, GRID_SIZE, GRID_SIZE);
  setLevel(0);
  mobs.push_back(new Snake(200, 32*GRID_SIZE, false));
  mobs.push_back(new Snake(400, 32*GRID_SIZE, false));
  mobs.push_back(new Snake(200, 29*GRID_SIZE, false));
  mobs.push_back(new Snake(600, 29*GRID_SIZE, true));
  mobs.push_back(new Snake(500, 26*GRID_SIZE, true));
}

void gameCleanup() {
  delete spriteSheet;
  delete spriteSheetSnake;
  delete spriteSheetFireball;
  delete spriteSheetFont;
  delete spriteSheetShaman;
  delete tilemapFloor;
  delete tileset;
  delete tilesetSnake;
  delete tilesetFireball;
  delete tilesetFont;
  delete tilesetShaman;
  delete sky;
}

