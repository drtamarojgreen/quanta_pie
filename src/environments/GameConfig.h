#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <istream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Gauntlet.h"
#include "../players/KeyBindings.h"

/**
 * @class ConfigError
 * @brief Thrown when config.txt cannot be read. The message names the line and the problem.
 */
class ConfigError : public std::runtime_error {
public:
    explicit ConfigError(const std::string& message) : std::runtime_error(message) {}
};

/**
 * @struct GameConfig
 * @brief Settings read from config.txt. Every setting is optional.
 *
 *   # Lines starting with # are comments
 *   data_directory = sql          # Where the gauntlet_*.csv tables are
 *   save_directory = .            # Where save slots are written
 *   background = balanced         # balanced, steadfast, swift or grounded
 *   mode = normal                 # normal, or hardcore (permadeath)
 *   tutorial = on                 # One-time tips
 *   color = on                    # Coloured text
 *   log_file = quantapie.log      # Write every game event to a file (off when empty)
 *   profile = off                 # Time the game's systems and write the report to the log
 *   key.k = move_north            # Rebind a key (see the help screen for command names)
 */
struct GameConfig {
    static const char* const kDefaultFile;

    std::string data_directory = "sql";
    GameOptions options;
    bool color = true;
    std::string log_file;
    bool profile = false;
    std::vector<std::pair<char, Command>> bindings;

    /**
     * @throws ConfigError naming the first bad line.
     */
    static GameConfig parse(std::istream& in);

    /**
     * @brief Reads a config file; a missing file gives the defaults.
     * @throws ConfigError if the file exists but has a bad line.
     */
    static GameConfig loadFile(const std::string& path);

    void applyBindings(KeyBindings& keys) const;
};

#endif // GAME_CONFIG_H
