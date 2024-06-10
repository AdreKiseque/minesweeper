#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "minesweeper.h"

#ifdef _WIN32
#include <windows.h>
#endif

typedef struct
{
    int mines;
    int tiles;
} clipboard;

const int MAX_SIZE = 99;
int length, height;
enum GameState status;
GameInfo info;

clipboard inspection(tile field[length][height], int x, int y);
void cascade(tile field[length][height], int x, int y);
void gameOver(tile field[length][height], int bombX, int bombY);
int commission(int order);

int main(int argc, char *argv[])
{
    srand(time(NULL));
    // Check for custom settings, set 8x8 field by default
    int orderedMines = -1;
    switch (argc)
    {
        case 1:
            length = 8, height = 8;
            break;
        case 2:
            length = atoi(argv[1]);
            height = atoi(argv[1]);
            break;
        case 4:
            orderedMines = atoi(argv[3]);
            // Fallthrough
        case 3:
            length = atoi(argv[1]);
            height = atoi(argv[2]);
            break;
        default:
            printf("whoah there buddy\n");
            return 1;
    }
    if (length < 4 || height < 4)
    {
        printf("bad values, mate\n");
        return 1;
    }
    if (length > MAX_SIZE || height > MAX_SIZE)
    {
        printf("triple digits are out of our budget\n");
        return 1;
    }
    tile field[length][height];
    setup(field, orderedMines, -1);

    // Render the game (graphics.c)
    render(field);
}

void setup(tile field[length][height], int clickX, int clickY)
{
    /* Ok this part's a little funky so bear with me. Basically when the function is called with -1 as the last parameter
    (sentinel value), clickX will be the ordered mines dude, and the function will just initialize the field and get the
    quota. When called with 2 extra parameters, clickX will be the x position of the starting click and clickY the y
    position, and the mines will be placed.*/

    static int quota = 0;
    if (clickY == -1)
    {
        quota = commission(clickX);
        // Initialize the tiles
        for (int x = 0; x < length; x++)
        {
            for (int y = 0; y < height; y++)
            {
                field[x][y].mine = false;
                field[x][y].blown = false;
                field[x][y].revealed = false;
                field[x][y].flagged = false;
                field[x][y].reserved = false;
            }
        }
        return;
    }

    // Initialize time
    info.gameTime = 0;

    // Reserve the space around initial click
    for (int xOffset = -1; xOffset < 2; xOffset++)
    {
        for (int yOffset = -1; yOffset < 2; yOffset++)
        {
            if ((clickX + xOffset >= 0 && clickX + xOffset <= length - 1) &&
                (clickY + yOffset >= 0 && clickY + yOffset <= height - 1))
            {
                field[clickX + xOffset][clickY + yOffset].reserved = true;
            }
        }
    }

    // Plant the bombs
    while (info.totalMines < quota)
    {
        for (int x = 0; x < length && info.totalMines < quota; x++)
        {
            for (int y = 0; y < height && info.totalMines < quota; y++)
            {
                if (!field[x][y].mine && !field[x][y].reserved)
                {
                    bool BOMB = !(rand() % 6);
                    field[x][y].mine = BOMB;
                    info.totalMines += BOMB;
                }
            }
        }
    }

    // Check for invalid arrangements and assign numbers
    for (int x = 0; x < length; x++)
    {
        for (int y = 0; y < height; y++)
        {
            clipboard report = inspection(field, x, y);
            if (report.mines == report.tiles)
            {
                // Don't tell the commissioner
                field[x][y].mine = false;
                info.totalMines--;
                // Ninesweeper.....
                report.mines--;
            }
            field[x][y].threats = report.mines;
        }
    }
    status = ACTIVE;
}

clipboard inspection(tile field[length][height], int x, int y)
{
    int mines = 0;
    int count = 0;
    for (int xOffset = -1; xOffset < 2; xOffset++)
    {
        for (int yOffset = -1; yOffset < 2; yOffset++)
        {
            if ((x + xOffset >= 0 && x + xOffset <= length - 1) &&
                (y + yOffset >= 0 && y + yOffset <= height - 1))
            {
                mines += field[x + xOffset][y + yOffset].mine;
                count++;
            }
        }
    }
    clipboard report;
    report.mines = mines;
    report.tiles = count;
    return report;
}

// Declared in header
void dig(tile field[length][height], int x, int y)
{
    field[x][y].revealed = true;
    info.tilesDug++;
    if (field[x][y].mine)
    {
        // BOMB
        gameOver(field, x, y);
    }
    else if (field[x][y].threats == 0)
    {
        cascade(field, x, y);
    }
    if (status == ACTIVE && info.tilesDug == length * height - info.totalMines)
    {
        status = WON;
    }
}

void cascade(tile field[length][height], int x, int y)
{
    for (int xOffset = -1; xOffset < 2; xOffset++)
    {
        for (int yOffset = -1; yOffset < 2; yOffset++)
        {
            if (x + xOffset >= 0 && x + xOffset <= length - 1 &&
                y + yOffset >= 0 && y + yOffset <= height - 1 &&
                !field[x + xOffset][y + yOffset].revealed)
            {
                dig(field, x + xOffset, y + yOffset);
            }
        }
    }
}

void gameOver(tile field[length][height], int bombX, int bombY)
{
    status = LOST;
    field[bombX][bombY].blown = true;
}

int commission(int order)
{
    int quota;
    int fieldSize = length * height;
    if (0 < order && order <= fieldSize / 2 && order <= fieldSize - 9)
    {
        quota = order;
    }
    else
    {
        if (order == 0 || order > fieldSize / 2)
        {
            printf("We'll take it from here, buddy\n");
        }
        if (fieldSize <= 100)
        {
            quota = round(sqrt(fieldSize));
        }
        else if (100 < fieldSize && fieldSize <= 256)
        {
            quota = 10 + ((fieldSize - 100) * (40 - 10)) / (256 - 100);
        }
        else
        {
            quota = 40 + ((fieldSize - 256) * (99 - 40)) / (480 - 256);
        }
    }
    return quota;
}

#ifdef __unix__
int endTimer()
{
    static bool started;
    static struct timespec startTime;
    struct timespec endTime;
    if (!started)
        {
            clock_gettime(CLOCK_REALTIME, &startTime);
            started = true;
        }
    clock_gettime(CLOCK_REALTIME, &endTime);
    return (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_nsec - startTime.tv_nsec) / 1e9;
}
#elif _WIN32
int endTimer()
{
    static bool started;
    static DWORD startTime;
    DWORD endTime;
    if (!started)
        {
            startTime = GetTickCount();
            started = true;
        }
    endTime = GetTickCount();
    return (endTime - startTime) / 1000;
}
#endif