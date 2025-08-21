#ifndef CONSOLE_H
#define CONSOLE_H

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
};

#endif // CONSOLE_H
