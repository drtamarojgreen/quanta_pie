#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>
#include <fstream>
#include <memory> // For std::unique_ptr
#include <windows.h> // Required for HANDLE
#include "Player.h"
#include "Room.h"
#include "GameSession.h"
#include "Score.h"
#include "Character.h" // Include full definition of Character
#include "Challenge.h" // Include Challenge definition

extern volatile sig_atomic_t g_signal_received; // Declare global signal flag

/**
 * @class Game
 * @brief Manages the main game loop, world creation, and player interaction.
 *
 * This class is the central hub of the Quanta_Pie game. It initializes the
 * game world, contains the main game loop, processes player input, and
 * tracks the overall game state.
 */
class Game {
public:
    /**
     * @brief Constructs a new Game object.
     */
    Game();

    /**
     * @brief Constructs a new Game object with a custom SQL file.
     */
    Game(const std::string& sql_file_path);

    /**
     * @brief Destroys the Game object, cleaning up allocated resources.
     */
    ~Game() = default; // With unique_ptr, default destructor is fine

    /**
     * @brief Starts the game, displaying the welcome message and beginning the main loop.
     */
    void start();

private:
    void createWorld(const std::string& sql_file_path);
    void gameLoop();
    void processInput(const std::string& input);
    void printWelcomeMessage();
    void printHelp();
    void loadDataFromCSV();
    std::vector<std::string> getRoomInfoLines(); // Modified to return lines
    std::vector<std::string> getSidePanelLines(); // Modified to return lines
    void displayGameScreen(); // New function to display combined screen

    std::unique_ptr<Player> player; // The main player character
    std::vector<std::unique_ptr<Room>> allRooms;
    std::vector<std::unique_ptr<Player>> allPlayers;
    std::vector<std::unique_ptr<GameSession>> allGameSessions;
    std::vector<std::unique_ptr<Score>> allScores;
    std::vector<std::unique_ptr<Character>> allCharacters;
    bool gameOver;
    HANDLE hConsole; // Console handle for advanced console manipulation
    std::unique_ptr<Challenge> current_challenge; // The currently active CBT challenge
};

#endif // GAME_H