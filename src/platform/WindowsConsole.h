#ifndef WINDOWS_CONSOLE_H
#define WINDOWS_CONSOLE_H

#include "Console.h"
#include <windows.h> // Required for HANDLE and other Windows API types

/**
 * @class WindowsConsole
 * @brief Implements the Console interface for the Windows platform.
 *
 * This class uses the Windows Console API to provide console manipulation
 * functionality, such as clearing the screen, setting the cursor position,
 * and reading single characters from the input.
 */
class WindowsConsole : public Console {
public:
    /**
     * @brief Constructs a new WindowsConsole object.
     *
     * Initializes the console handle.
     */
    WindowsConsole();

    /**
     * @brief Destroys the WindowsConsole object.
     */
    ~WindowsConsole() override = default;

    /**
     * @brief Clears the entire console screen.
     */
    void clear() override;

    /**
     * @brief Sets the cursor position to the specified coordinates.
     * @param x The column (horizontal position).
     * @param y The row (vertical position).
     */
    void setCursorPosition(int x, int y) override;

    /**
     * @brief Reads a single character from the console without waiting for a newline.
     * @return The ASCII value of the character read.
     */
    int getChar() override;

private:
    HANDLE hConsole; // Handle to the console screen buffer
};

#endif // WINDOWS_CONSOLE_H
