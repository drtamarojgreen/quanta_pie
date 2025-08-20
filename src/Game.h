#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>
#include <fstream>
#include "Player.h"
#include "Room.h"
#include "GameSession.h"
#include "Score.h"

// Forward declarations are no longer needed for Player and Room
// as we are now including the full headers.
// Forward declarations
class Player;
class Map;

class Game {
public:
    Game();
    ~Game();
    void start();

private:
    void createWorld();
    void gameLoop();
    void render();
    void printWelcomeMessage();
    void printHelp();
    void loadDataFromSQL(const std::string& filename);

    Player* player; // The main player character
    std::vector<Room*> allRooms;
    std::vector<Player*> allPlayers;
    std::vector<GameSession*> allGameSessions;
    std::vector<Score*> allScores;

    Player* player;
    Map* map;
    bool gameOver;
    int score;

    // Screen dimensions
    static const int screenWidth = 80;
    static const int screenHeight = 24;
};

#endif // GAME_H
