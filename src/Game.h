#ifndef GAME_H
#define GAME_H

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <csignal> // For sig_atomic_t
#include "environments/GameConfig.h"
#include "environments/Gauntlet.h"
#include "environments/Hud.h"
#include "environments/Profiler.h"

// Forward declaration for the Console class to avoid including platform-specific headers
class Console;

extern volatile sig_atomic_t g_signal_received; // Declare global signal flag

/**
 * @class Game
 * @brief The console frontend: a welcome menu, then draws the gauntlet and feeds it key presses.
 *
 * Settings come from config.txt (see GameConfig); the rooms, layouts and spawn points are
 * loaded from the CSV tables in the data directory. The same Game runs on Windows
 * (WindowsConsole) and Linux (PosixConsole); only the console differs. Frames are redrawn
 * line by line, only where they changed.
 */
class Game {
public:
    /**
     * @brief Constructs a new Game object from config.txt, if there is one.
     * @throws ConfigError or WorldDataError if the settings or the world data are invalid.
     */
    Game();

    /**
     * @brief Constructs a new Game object, loading the world from a data directory.
     * @throws ConfigError or WorldDataError if the settings or the world data are invalid.
     */
    Game(const std::string& data_directory);

    /**
     * @brief Constructs a Game with given settings, console and output stream.
     * @throws WorldDataError if the world data is missing or invalid.
     */
    Game(const GameConfig& config, std::unique_ptr<Console> console, std::ostream& output = std::cout);

    /**
     * @brief Destroys the Game object, cleaning up allocated resources.
     */
    ~Game(); // Defined in .cpp to handle unique_ptr to incomplete type

    /**
     * @brief Starts the game: the welcome menu, then the main loop.
     */
    void start();

    const Gauntlet& getGauntlet() const;
    const Profiler& getProfiler() const;

private:
    bool runWelcomeMenu(); // Returns false if input ends before the game begins
    void gameLoop();
    void displayGameScreen();
    void writeStyled(const Hud::StyledLine& line);
    void processKey(int key);
    void logEvent(const GameEvent& event);

    GameConfig config;
    std::unique_ptr<Console> console; // Platform-agnostic console interface
    std::ostream& output;
    Profiler profiler;
    Gauntlet gauntlet;
    std::ofstream log;
    std::vector<Hud::StyledLine> frame; // What is on screen now
    bool shake_next_frame;
};

#endif // GAME_H
