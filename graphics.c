#include "tigr/tigr.h"
#include "minesweeper.h"

// Very cool of TIGR to not bother abstracting platform-specific mouse values 👍
#ifdef __unix__
#define MOUSE_LEFT 1
#define MOUSE_RIGHT 2
#elif _WIN32
#define MOUSE_LEFT 1
#define MOUSE_RIGHT 4
#endif

typedef struct
{
    int x;
    int y;
    int b;
    int last;
} Mouse;

typedef struct
{
    Tigr *BOMB;
    Tigr *flag;
    Tigr *clear;
    Tigr *wrong;
    Tigr *clock;
    Tigr *uhOh1;
    Tigr *uhOh2;
    Tigr *boom;
    Tigr *ash;
}   SpriteSheet;

void drawTile(Tigr *screen, int x, int y, tile field[length][height], Mouse mouse, SpriteSheet sprites);
void drawBorder(Tigr *screen, tile field[length][height], SpriteSheet sprites);
TPixel numColour(int num);
Tigr *explode(SpriteSheet sprites);

// Tile stuff still requires lots of manual adjusting
const int TILE_SIZE   = 15;
const int BORDER_HOR  = 20;
const int BORDER_VERT = 30;

void render(tile field[length][height])
{
    Tigr *screen = tigrWindow(length * TILE_SIZE + BORDER_HOR, height * TILE_SIZE + BORDER_VERT, "Minesweeper", 0);
    SpriteSheet sprites;
    sprites.BOMB = tigrLoadImage("images/BOMB.png");
    sprites.flag = tigrLoadImage("images/flag.png");
    sprites.clear = tigrLoadImage("images/clear.png");
    sprites.wrong = tigrLoadImage("images/wrong.png");
    sprites.clock = tigrLoadImage("images/clock.png");
    sprites.uhOh1 = tigrLoadImage("images/uh_oh_1.png");
    sprites.uhOh2 = tigrLoadImage("images/uh_oh_2.png");
    sprites.boom = tigrLoadImage("images/boom.png");
    sprites.ash = tigrLoadImage("images/ash.png");

    while (!tigrClosed(screen))
    {
        tigrClear(screen, tigrRGB(160, 160, 170));

        // Get mouse state
        Mouse mouse;
        tigrMouse(screen, &mouse.x, &mouse.y, &mouse.b);

        // Draw field
        for (int x = 0; x < length; x++)
        {
            for (int y = 0; y < height; y++)
            drawTile(screen, x, y, field, mouse, sprites);
        }
        drawBorder(screen, field, sprites);
        mouse.last = mouse.b;
        tigrUpdate(screen);
    }

    // Guess what this part does
    tigrFree(sprites.BOMB);
    tigrFree(sprites.flag);
    tigrFree(sprites.clear);
    tigrFree(sprites.wrong);
    tigrFree(sprites.clock);
    tigrFree(sprites.uhOh1);
    tigrFree(sprites.uhOh2);
    tigrFree(sprites.boom);
    tigrFree(sprites.ash);
    tigrFree(screen);
}

void drawTile(Tigr *screen, int x, int y, tile field[length][height], Mouse mouse, SpriteSheet sprites)
{
    int xPos = x * TILE_SIZE + BORDER_HOR / 2;
    int yPos = y * TILE_SIZE + BORDER_VERT - BORDER_HOR / 2;

    // Temporary shorthand for tile size so the code isn't nightmarishly long
    int TS = TILE_SIZE;
    // Offsets for numbers and icons and stuff need to be manually adjusted, though (for now...)

    // Foundation tile
    tigrFillRect(screen, xPos, yPos, TS+1, TS+1, tigrRGB(160, 160, 160));
    tigrRect    (screen, xPos, yPos, TS+1, TS+1, tigrRGB(80, 80, 80));

    if (!field[x][y].revealed)
    {
        for (int i = 1; i <= 2; i++)
        {
            // Covered tile (light shading)
            tigrLine(screen, xPos, yPos+i-1, xPos+TS-i, yPos+i-1, tigrRGB(240, 240, 240));
            tigrLine(screen, xPos+i-1, yPos+2, xPos+i-1, yPos+TS-i, tigrRGB(240, 240, 240));
            // Dark shading
            tigrLine(screen, xPos+i, yPos+TS-i, xPos+TS, yPos+TS-i, tigrRGB(80, 80, 80));
            tigrLine(screen, xPos+TS-i, yPos+i, xPos+TS-i, yPos+TS, tigrRGB(80, 80, 80));
        }
        // Shading stragglers
        tigrPlot(screen, xPos+TS-1, yPos, tigrRGB(160, 160, 160));
        tigrPlot(screen, xPos, yPos+TS-1, tigrRGB(160, 160, 160));

        // On game win, flag any remaining tiles
        if (status == WON && !field[x][y].flagged)
        {
            field[x][y].flagged = true;
            info.flagsPlaced++;
        }

        if (field[x][y].flagged)
        {
            if (status == LOST && !field[x][y].mine && endTimer() >= 2)
            {
                // borken flag....
                tigrBlitAlpha(screen, sprites.wrong, xPos+4, yPos+3, 0, 0, 12, 12, 100);
            }
            else
            {
                // Draw a little flag
                tigrBlitAlpha(screen, sprites.flag, xPos+4, yPos+3, 0, 0, 12, 12, 100);
            }
        }

        // Check the mouse
        if (xPos+1 < mouse.x && mouse.x < xPos+TS-2 &&
            yPos+1 < mouse.y && mouse.y < yPos+TS-2 && status == START)
        {
            if (mouse.b == MOUSE_LEFT && !field[x][y].flagged && !mouse.last)
            {
                setup(field, x, y);
                dig(field, x, y);
            }
        }

        else if (xPos+1 < mouse.x && mouse.x < xPos+TS-2 &&
            yPos+1 < mouse.y && mouse.y < yPos+TS-2 && status == ACTIVE)
        {
            if (mouse.b == MOUSE_LEFT && !field[x][y].flagged && !mouse.last)
            {
                dig(field, x, y);
            }
            else if (mouse.b == MOUSE_RIGHT && !mouse.last)
            {
                field[x][y].flagged = !field[x][y].flagged;
                if (field[x][y].flagged)
                {
                    info.flagsPlaced++;
                }
                else
                {
                    info.flagsPlaced--;
                }
            }
        }

        // Reveal unfound mines on loss
        if (status == LOST && !field[x][y].flagged && field[x][y].mine && endTimer() >= 2)
        {
            field[x][y].revealed = true;
        }
    }
    else
    {
        // Secrets revealed
        if (field[x][y].mine)
        {
            // BOMB
            if (!field[x][y].blown)
            {
                tigrBlitAlpha(screen, sprites.BOMB, xPos+2, yPos+2, 0, 0, 12, 12, 100);
            }
            else
            {
                tigrBlitAlpha(screen, explode(sprites), xPos+2, yPos+2, 0, 0, 12, 12, 100);
            }
        }
        else if (field[x][y].threats)
        {
            // Display the coloured number
            tigrPrint(screen, tfont, xPos+5, yPos+4, numColour(field[x][y].threats), "%d", field[x][y].threats);
        }
    // If nothing else, fall back to the empty foundation tile :)
    }
}

TPixel numColour(int num)
{
    TPixel colour;
    switch (num)
    {
        case 1:
            colour = tigrRGB(0, 0, 250);
            break;
        case 2:
            colour = tigrRGB(0, 128, 0);
            break;
        case 3:
            colour = tigrRGB(250, 0, 0);
            break;
        case 4:
            colour = tigrRGB(0, 0, 125);
            break;
        case 5:
            colour = tigrRGB(125, 0, 0);
            break;
        case 6:
            colour = tigrRGB(128, 128, 0);
            break;
        case 7:
            colour = tigrRGB(0, 0, 0);
            break;
        case 8:
            colour = tigrRGB(80, 80, 80);
    }
    return colour;
}

void drawBorder(Tigr *screen, tile field[length][height], SpriteSheet sprites)
{
    int screenLength = length * TILE_SIZE + BORDER_HOR;
    int screenHeight = height * TILE_SIZE + BORDER_VERT;
    int fieldStartX = BORDER_HOR / 2;
    int fieldStartY = BORDER_VERT - BORDER_HOR / 2;
    int fieldLength = TILE_SIZE * length;
    int fieldHeight = TILE_SIZE * height;
    int fieldEndX = fieldStartX + fieldLength;
    int fieldEndY = fieldStartY + fieldHeight;

    for (int i = 1; i <= 3; i++)
    {
        // Screen top, left
        tigrLine(screen, 0, i-1, screenLength-i, i-1, tigrRGB(240, 240, 255));
        tigrLine(screen, i-1, 3, i-1, screenHeight-i, tigrRGB(240, 240, 255));
        // Screen bottom, right
        tigrLine(screen, i, screenHeight-i, screenLength, screenHeight-i, tigrRGB(80, 80, 100));
        tigrLine(screen, screenLength-i, i, screenLength-i, screenHeight-3, tigrRGB(80, 80, 100));

        // Field top, left
        tigrLine(screen, fieldStartX-3, fieldStartY-i, fieldEndX+i-1, fieldStartY-i, tigrRGB(80, 80, 100));
        tigrLine(screen, fieldStartX-i, fieldStartY, fieldStartX-i, fieldEndY+(i-1), tigrRGB(80, 80, 100));
        // Field bottom, right
        tigrLine(screen, fieldStartX-(i-1), fieldEndY+(i-1), fieldEndX+3, fieldEndY+(i-1), tigrRGB(240, 240, 255));
        tigrLine(screen, fieldEndX+(i-1), fieldStartY-(i-1), fieldEndX+(i-1), fieldEndY, tigrRGB(240, 240, 255));
    }

    int textStartY = fieldStartY / 2 - tigrTextHeight(tfont, "0") / 2 + 2;
    // To align the counters, we need to start from the appropriate fraction and subtract half the total size, which is the length of the printed text + the width of the icon + the spacing between the icon and text.
    int flagStartX = (screenLength * 2 / 7) - (tigrTextWidth(tfont, "000") + (6 + 2)) / 2;
    int timeStartX = (screenLength * 5 / 7) - (tigrTextWidth(tfont, "000") + (9 + 2)) / 2;
    // Then for the text, we'll just add the width of the icon + the spacing.

    if (status == WON)
    {
        tigrBlitAlpha(screen, sprites.clear, flagStartX, textStartY, 0, 0, 9, 9, 100);
    }
    else
    {
        tigrBlitAlpha(screen, sprites.flag, flagStartX, textStartY, 0, 0, 9, 9, 100);
    }
    tigrPrint(screen, tfont, flagStartX + (6 + 2), textStartY, tigrRGB(255, 255, 255), "%02d", info.totalMines - info.flagsPlaced);

    if (status == ACTIVE)
    {
        static float timer = 0;
        timer += tigrTime();
        info.gameTime = timer;
    }

    tigrBlitAlpha(screen, sprites.clock, timeStartX, textStartY, 0, 0, 9, 9, 100);
    tigrPrint(screen, tfont, timeStartX + (9 + 2), textStartY, tigrRGB(255, 255, 255), "%03d", info.gameTime);
}

Tigr *explode(SpriteSheet sprites)
{
    static float timer = 0;
    if (timer <= 1)
    {
        timer += tigrTime();
    }

    if (timer > 1)
    {
        return sprites.ash;
    }
    else if (timer > .5)
    {
        return sprites.boom;
    }
    else if (timer > .4)
    {
        return sprites.uhOh2;
    }
    else
    {
        return sprites.uhOh1;
    }
}
