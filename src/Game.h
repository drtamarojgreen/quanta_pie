#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>
#include <fstream>

// Forward declarations
class Player;
class Room;
class Character;

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
     * @brief Destroys the Game object, cleaning up allocated resources.
     */
    ~Game();

    /**
     * @brief Starts the game, displaying the welcome message and beginning the main loop.
     */
    void start();

private:
    void createWorld();
    void gameLoop();
    void processInput(const std::string& input);
    void printWelcomeMessage();
    void printHelp();
    void loadDataFromSQL(const std::string& filename);
    void printCurrentRoomInfo();

    Player* player;
    std::vector<Room*> allRooms;
    std::vector<Character*> allCharacters;
    bool gameOver;
};

#endif // GAME_H
