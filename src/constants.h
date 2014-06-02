#ifndef CONSTANTS_H
#define CONSTANTS_H

const int SCREEN_WIDTH  = 640;
const int GRID_HEIGHT = 480;

const int NUM_ROWS = 8;
const int NUM_COLUMNS = 8;
const int ROW_HEIGHT = GRID_HEIGHT / NUM_ROWS;
const int COLUMN_WIDTH = SCREEN_WIDTH / NUM_COLUMNS;
const int MAX_DROP_SPEED = 10;
const int SWAP_SPEED = 6;
const int NUM_GEM_TYPES = 5;
const int DRAG_DEAD_ZONE = 30;
const int HUD_HEIGHT = 30;
const int TIMEOUT = 60;
const int GAME_OVER_TIMEOUT = 3;

const int GAMESTATE_INIT = 0;
const int GAMESTATE_CHECKDROP = 1;
const int GAMESTATE_DROP = 2;
const int GAMESTATE_AWAIT_INPUT = 3;
const int GAMESTATE_SWAP_LEFT = 4;
const int GAMESTATE_SWAP_RIGHT = 5;
const int GAMESTATE_SWAP_UP = 6;
const int GAMESTATE_SWAP_DOWN = 7;
const int GAMESTATE_CHECK_MATCH = 8;
const int GAMESTATE_INITIAL_CHECK_MATCH = 9;
const int GAMESTATE_SWAP_HORIZONTAL = 10;
const int GAMESTATE_ADD_GEMS = 11;
const int GAMESTATE_TITLE_SCREEN = 12;
const int GAMESTATE_GAME_OVER = 13;

const int GEMSTATE_IDLE = 0;
const int GEMSTATE_FALL = 1;
const int GEMSTATE_DEAD = 2;
const int GEMSTATE_BOUNCE = 3;
const int GEMSTATE_MOVE_LEFT = 4;
const int GEMSTATE_MOVE_RIGHT = 5;
const int GEMSTATE_MOVE_UP = 6;
const int GEMSTATE_MOVE_DOWN = 7;
const int GEMSTATE_ENTER = 8;

const int ROW = 0;
const int COLUMN = 1;

#endif