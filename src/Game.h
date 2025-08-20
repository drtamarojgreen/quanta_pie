#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>

// Forward declarations
class Player;
class Room;

class Game {
public:
    Game();
    ~Game();

    void start();

private:
    void createWorld();
    void gameLoop();
    void processInput(const std::string& input);
    void printWelcomeMessage();
    void printHelp();

    Player* player;
    std::vector<Room*> allRooms;
    bool gameOver;
};

#endif // GAME_H
