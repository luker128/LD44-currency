#pragma once

  const int MAP_W = 30;
  const int MAP_H = 33;

  const int TILE_GRASS = 0;
  const int TILE_FREE = 1;
  const int TILE_WALL = 2;
  const int TILE_STONE = 3;
  const int TILE_DIAMOND = 4;
  const int TILE_PLAYER = 5;

//extern int level[];
void setTile(int x, int y, int newTile);
bool isTileSolid(int tile);
bool isTileLadder(int tile);
int tileAt(int x, int y);
void setLevel(int l);

