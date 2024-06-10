# Minesweeper in C

**Video:** https://youtu.be/dMIG2Q7ONvg

Minesweeper remade in C for a personal project. I tried to do everything as manually and low-level as I could, I'm pretty proud of it! I used erkkah's [TIGR](https://github.com/erkkah/tigr) library for the graphics, with some small modifications (edited the default font, few small memory patches... though it's still not perfect). Some features I'd have liked to implement but didn't get around to were tracked high scores/best times, being able to reset or reconfigure without having to relaunch the game, and sound effects. If I ever come back to this, I'll probably add those in. But overall I'm quite happy with how it turned out :)

## Installation
Take the source and compile it :) Should run on any kind of PC—Windows, Mac, Linux, etc... probably.

If it doesn't work just fiddle with it idk. Make sure you've got all the right stuff installed and all. I had a time when it wouldn't compile, I switched compilers then switched back and then it did. Trust the process.

## Usage
Launch the program with `./minesweeper`. The program takes 3 optional command-line arguments, integer values—inputting 1 will launch with a square field with sides of that size, with 2, the first value is used for the length and the second for height. Inputting a third argument will set a custom mine count, else the target count will be calculated based on the size of the field. Field dimensions must be between 4 and 99, and mine count must be between 1 and half the total field size (or field size − 9, if you're playing in 4 × 4). If no dimensions are given an 8 × 8 field will be used.

As stated earlier, I haven't implemented a way to restart from within the game, so you'll just have to close and relaunch. Think of it like getting up to press the reset button on an old game console :)

The game has a few systems in place to make playing more pleasant. Your first click will always be safe and reveal all tiles around it, and there's guarenteed to not be a mine completely surrounded by other mines or the border. I wasn't able to eliminate 50/50s entirely, however, but I think I still did a pretty good job!

Enjoy! :)