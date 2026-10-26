#ifndef POSIX_CONSOLE_H
#define POSIX_CONSOLE_H

#include "Console.h"

/**
 * @class PosixConsole
 * @brief Implements the Console interface for Linux and other POSIX terminals.
 *
 * Uses ANSI escape codes for the screen and colours, and reads one key at a time without echo.
 * Arrow and navigation keys are reported the way Windows' _getch() reports them:
 * the prefix 224 followed by the scan code on the next call, so the game handles
 * both platforms the same way. getChar() returns -1 at end of input.
 */
class PosixConsole : public Console {
public:
    PosixConsole();
    ~PosixConsole() override = default;

    void clear() override;
    void setCursorPosition(int x, int y) override;
    int getChar() override;
    int getConsoleWidth() const override;
    void setColor(ConsoleColor color) override;

private:
    int pending_scan_code; // Second half of a translated arrow key, or -1
};

#endif // POSIX_CONSOLE_H
