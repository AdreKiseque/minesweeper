#include <stdbool.h>

typedef struct
{
    bool revealed;
    bool flagged;
    bool mine;
    bool blown;
    bool reserved;
    int threats;
} tile;

extern int length, height;

void setup(tile field[length][height], int clickX, int clickY);
void dig(tile field[length][height], int x, int y);
void render(tile field[length][height]);

enum GameState {
  START,
  ACTIVE,
  LOST,
  WON
};

extern enum GameState status;

typedef struct
{
  int totalMines;
  int tilesDug;
  int flagsPlaced;
  int gameTime;
} GameInfo;

extern GameInfo info;

int endTimer();
