#include "WindowsConsole.h"
#include <conio.h>   // Required for _getch()
#include <windows.h> // Required for Windows console API

WindowsConsole::WindowsConsole() : hConsole(GetStdHandle(STD_OUTPUT_HANDLE)) {}

void WindowsConsole::setCursorPosition(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(hConsole, coord);
}

void WindowsConsole::clear() {
    // This implementation is based on the ClearConsoleRegion function from Game.cpp
    // It clears a 120x50 region, which should be sufficient for the game screen.
    DWORD count;
    COORD coord = { 0, 0 };
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    GetConsoleScreenBufferInfo(hConsole, &csbi);

    // Fill the region with spaces
    FillConsoleOutputCharacter(hConsole, (TCHAR) ' ', 120 * 50, coord, &count);

    // Fill the region with the current background color
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, 120 * 50, coord, &count);

    // Set the cursor to the top-left of the region
    setCursorPosition(0, 0);
}

int WindowsConsole::getChar() {
    return _getch();
}
