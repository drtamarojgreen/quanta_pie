#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>

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

    Player* player;
    Map* map;
    bool gameOver;
    int score;

    // Screen dimensions
    static const int screenWidth = 80;
    static const int screenHeight = 24;
};

#endif // GAME_H
