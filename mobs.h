class Mob {
  public:
    Mob() : dead(false) {}
    virtual void draw() = 0;
    virtual void update() = 0;
    virtual void checkCollision(int, int) = 0;
    bool isDead() { return dead; }
  protected:
    bool dead;
};

class Snake : public Mob {
  public:
    static const int MAX_LEFT = 10;
    static const int MAX_RIGHT = MAP_W*GRID_SIZE - 10;
    Snake(int x, int y, bool flipped)
      : x(x), y(y), flipped(flipped), frame(0), nextFrameIn(10) {}
    void draw() override {
      if (flipped) {
        spriteSheetSnake->drawSpriteFlipped(x+16+32+rumble_x, y-32-32+scroll_y+rumble_y, frame);
      } else {
        spriteSheetSnake->drawSprite(x-16-32+rumble_x, y-32-32+scroll_y+rumble_y, frame);
      }
//      print(adjustPosition(), y+scroll_y, "+");
    }
    void update() override {
      nextFrameIn--;
      if (nextFrameIn == 0) {
        frame++;
        nextFrameIn = 10;
        if (frame == 7) {
          frame = 1;
          if (flipped) {
            if (isTileSolid(tileAt((x-11)/GRID_SIZE, (y+1)/GRID_SIZE)) && x > MAX_LEFT) {
              x -= 11;
            } else {
              flipped = !flipped;
            }
          } else {
            if (isTileSolid(tileAt((x+11)/GRID_SIZE, (y+1)/GRID_SIZE)) && x < MAX_RIGHT) {
              x += 11;
            } else {
              flipped = !flipped;
            }
          }
        }
      }
    }
    int adjustPosition() {
      if (!flipped) {
        return x - 6;
      }
      return x+6;
    }
    void checkCollision(int player_x, int player_y) override {
      int dx = adjustPosition() - player_x;
      int dy = y - player_y;
      if (dy >= 0 && dy <= 20 && dx > -20 && dx < 20) {
        player_vx = 4 * (flipped ? -1 : +1);
        player_vy = -4;
        player_knockback = 10;
        player_knockdown = 40;
        player_knockback_flipped = flipped;
      }

    }
  private:
    int x;
    int y;
    bool flipped;
    int frame;
    int nextFrameIn;
};

class Rock: public Mob {
  public:
    static const int FIRST_FRAME = 2;
    static const int LAST_FRAME = 6;
    Rock(int x, int y)
      : x(x), y(player_y-300), target_y(y), frame(FIRST_FRAME), nextFrameIn(10), rolling(false) {
      origin_x = x;
      origin_y = y;
    }
    void draw() override {
      spriteSheetFireball->drawSprite(x-16-32, y-32-32+scroll_y, frame);
//      print(adjustPosition(), y+scroll_y, "+");
    }
    int getLevel() {return origin_y;}
    int getOrigin() {return origin_x;}
    void update() override {
      nextFrameIn--;
      if (nextFrameIn == 0) {
        frame++;
        nextFrameIn = 10;
        if (frame == LAST_FRAME) {
          frame = FIRST_FRAME;
          x+=48;
          if (rolling) {
            int tile = tileAt((adjustPosition()+10)/GRID_SIZE, (y+5)/GRID_SIZE);
            if (!isTileSolid(tile) && tile != 9) {
              rolling = false;
              target_y = 10000;
            }
          }
        }
      }
      if (!rolling) {
        y+=3;
        if (y >= target_y) {
          rolling = true;
          y = target_y;
        }
      }
      if (y >1050) {
        dead = true;
      }
    }
    int adjustPosition() {
      int f = frame - FIRST_FRAME;
      return x - (34-(f * 10));
    }
    void checkCollision(int player_x, int player_y) override {
      int dx = adjustPosition() - player_x;
      int dy = y - player_y;
      const int TOP = 32;
      bool playerLanded = false;
      if (dx > -20 && dx < 20) {
        if (dy >= TOP && dy < TOP+4 && player_vy >= 0) {
          if (!hasPlayerOnTop) {
            hasPlayerOnTop = true;
            riderOffset = dx;
          }
          playerLanded = true;
          player_riding = true;
          player_jumping = false;
          ::player_x = adjustPosition() - riderOffset;
          ::player_y = y -TOP;
          player_vy = 0;
          player_vx = 0;
        } else {
          if (dy >= -5 && dy <= 20) {
            player_vx = 4;
            player_vy = -4;
            player_knockback = 10;
            player_knockdown = 40;
          }
        }
      }
      if (!playerLanded) {
        hasPlayerOnTop = false;
      }
    }
  private:
    int x;
    int y;
    int target_y;
    int frame;
    int nextFrameIn;
    bool rolling;
    int origin_x;
    int origin_y;
    bool hasPlayerOnTop = false;
    int riderOffset = 0;
};


class Fireball: public Mob {
  public:
    Fireball(int x, int y)
      : x(x), y(y), frame(0), nextFrameIn(10) {}
    void draw() override {
      spriteSheetFireball->drawSprite(x-32, y-32-32+scroll_y, frame);
//      print(x, y+scroll_y, "+");
    }
    void update() override {
      y++;
      nextFrameIn--;
      if (nextFrameIn == 0) {
        frame++;
        nextFrameIn = 10;
        if (frame == 2) {
          frame = 0;
        }
      }
    }
    void checkCollision(int player_x, int player_y) override {
      int dx = x - player_x;
      int dy = y - player_y;
      if (dy >= -20 && dy <= 20 && dx > -20 && dx < 20) {
        player_vx = 4;
        player_vy = -4;
        player_knockback = 10;
        player_knockdown = 40;
      }
    }
  private:
    int x;
    int y;
    int frame;
    int nextFrameIn;
};


class Shaman: public Mob {
  public:
    Shaman(int x, int y)
      : x(x), y(y), frame(0), nextFrameIn(10) {}
    void draw() override {
      spriteSheetShaman->drawSprite(x-32, y-32-32+scroll_y, frame);
    }
    void update() override {
    }
    void checkCollision(int player_x, int player_y) override {
    }
  private:
    int x;
    int y;
    int frame;
    int nextFrameIn;
};


