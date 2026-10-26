#include "WindowsConsole.h"
#include <conio.h>   // Required for _getch()
#include <windows.h> // Required for Windows console API

WindowsConsole::WindowsConsole() : hConsole(GetStdHandle(STD_OUTPUT_HANDLE)), defaultAttributes(0) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        defaultAttributes = csbi.wAttributes;
    } else {
        defaultAttributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }
}

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

int WindowsConsole::getConsoleWidth() const {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

void WindowsConsole::setColor(ConsoleColor color) {
    // Keep the background bits; replace only the foreground colour.
    WORD background = defaultAttributes & 0xF0;
    WORD foreground = defaultAttributes & 0x0F;
    switch (color) {
        case ConsoleColor::Default: break;
        case ConsoleColor::Red:     foreground = FOREGROUND_RED | FOREGROUND_INTENSITY; break;
        case ConsoleColor::Green:   foreground = FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
        case ConsoleColor::Yellow:  foreground = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
        case ConsoleColor::Blue:    foreground = FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
        case ConsoleColor::Magenta: foreground = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
        case ConsoleColor::Cyan:    foreground = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
        case ConsoleColor::White:   foreground = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
    }
    SetConsoleTextAttribute(hConsole, background | foreground);
}
