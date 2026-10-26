#ifndef CONSOLE_H
#define CONSOLE_H

/**
 * @enum ConsoleColor
 * @brief Foreground colours every console can show.
 */
enum class ConsoleColor {
    Default,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White
};

/**
 * @class Console
 * @brief An abstract base class for platform-specific console operations.
 *
 * This class defines a common interface for console manipulation, such as
 * clearing the screen, setting the cursor position, and reading a single
 * character from the input. This allows the game logic to be decoupled
 * from the underlying console API.
 */
class Console {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~Console() = default;

    /**
     * @brief Clears the entire console screen.
     */
    virtual void clear() = 0;

    /**
     * @brief Sets the cursor position to the specified coordinates.
     * @param x The column (horizontal position).
     * @param y The row (vertical position).
     */
    virtual void setCursorPosition(int x, int y) = 0;

    /**
     * @brief Reads a single character from the console without waiting for a newline.
     * @return The ASCII value of the character read.
     */
    virtual int getChar() = 0;

    /**
     * @brief Gets the width of the console window.
     * @return The width of the console window in characters.
     */
    virtual int getConsoleWidth() const = 0;

    /**
     * @brief Sets the colour of text written after this call. Consoles without colour ignore it.
     * @param color The foreground colour; Default restores the console's normal colour.
     */
    virtual void setColor(ConsoleColor color) { (void)color; }
};

#endif // CONSOLE_H
