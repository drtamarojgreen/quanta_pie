#ifndef KEY_BINDINGS_H
#define KEY_BINDINGS_H

#include <array>
#include <istream>
#include <string>
#include <vector>
#include "../objects/Position.h"

/**
 * @enum Command
 * @brief Every action a player can trigger from the keyboard.
 */
enum class Command {
    None,
    MoveNorth,
    MoveNorthEast,
    MoveEast,
    MoveSouthEast,
    MoveSouth,
    MoveSouthWest,
    MoveWest,
    MoveNorthWest,
    Wait,
    Look,
    Dash,
    Clear,
    Pause,
    ShowInventory,
    ShowStats,
    ShowBestiary,
    ShowHelp,
    Save,
    Load,
    NewGame,
    Quit
};

/**
 * @class KeyBindings
 * @brief Maps single keys to commands. Bindings can be changed at runtime or loaded from a config.
 *
 * Defaults put the eight movement keys around 's' (q w e / a d / z x c) with 's' to wait.
 * A key maps to one command; a command may have several keys.
 */
class KeyBindings {
public:
    KeyBindings(); // Starts with the default bindings

    void resetToDefaults();
    void bind(char key, Command command);
    void unbind(char key);
    Command commandFor(char key) const;
    std::vector<char> keysFor(Command command) const;

    /**
     * @brief Applies "key = command_name" lines. Blank lines and lines starting with '#' are ignored.
     * @return false, leaving the bindings unchanged, if any line is malformed.
     */
    bool load(std::istream& config);

    static std::string commandName(Command command);
    static Command commandFromName(const std::string& name);
    static bool directionFor(Command command, Direction& direction); // False if command is not a move

private:
    std::array<Command, 128> bindings; // Indexed by ASCII key for constant-time lookup
};

#endif // KEY_BINDINGS_H
