#include "Game.h"
#include "Player.h"
#include "Map.h"
#include <iostream>
#include <vector>
#include <cmath> // For std::cos, std::sin

// Game class members definitions for screen dimensions
const int Game::screenWidth;
const int Game::screenHeight;

Game::Game() : player(nullptr), map(nullptr), gameOver(false), score(0) {
    createWorld();
}

#include <string>
#include <vector>
#include <sstream>

// Helper function to extract values from an SQL INSERT statement.
// This is a simple parser and not robust for general SQL.
std::vector<std::string> parseValues(const std::string& line) {
    std::vector<std::string> values;
    size_t start = line.find('(');
    size_t end = line.find(')');
    if (start == std::string::npos || end == std::string::npos) {
        return values;
    }
    std::string content = line.substr(start + 1, end - start - 1);
    std::stringstream ss(content);
    std::string value;
    while (std::getline(ss, value, ',')) {
        // Trim leading whitespace
        size_t first = value.find_first_not_of(" \t\n\r");
        if (std::string::npos != first) {
            value = value.substr(first);
        }
        // Trim trailing whitespace
        size_t last = value.find_last_not_of(" \t\n\r");
        if (std::string::npos != last) {
            value = value.substr(0, last + 1);
        }
        // Remove single quotes if they exist
        if (value.front() == '\'' && value.back() == '\'') {
            value = value.substr(1, value.length() - 2);
        }
        values.push_back(value);
    }
    return values;
}


Game::~Game() {
    // The 'player' pointer is a reference to an object within 'allPlayers',
    // so it should not be deleted separately.
    for (auto p : allPlayers) {
        delete p;
    }
    for (auto r : allRooms) {
        delete r;
    }
    for (auto gs : allGameSessions) {
        delete gs;
    }
    for (auto s : allScores) {
        delete s;
    }
}

void Game::createWorld() {
    // Load all game data from the SQL file
    loadDataFromSQL("sql/game_data.sql");

    // Assign the first loaded player as the main player character.
    if (!allPlayers.empty()) {
        player = allPlayers[0];
        // Assign the first loaded room as the starting location for the main player.
        if (!allRooms.empty()) {
            player->setCurrentRoom(allRooms[0]);
        }
    } else {
        // Fallback: if no players or rooms are loaded, create defaults.
        if (allRooms.empty()) {
            Room* defaultRoom = new Room("A non-descript, empty void.");
            allRooms.push_back(defaultRoom);
        }
        player = new Player(0, "Default Player", "unknown", allRooms[0]);
        allPlayers.push_back(player);
    }
}

void Game::loadDataFromSQL(const std::string& filename) {
    std::ifstream sqlFile(filename);
    if (!sqlFile.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(sqlFile, line)) {
        if (line.find("INSERT INTO players") != std::string::npos) {
            std::vector<std::string> values = parseValues(line);
            if (values.size() == 3) {
                allPlayers.push_back(new Player(std::stoi(values[0]), values[1], values[2], nullptr));
            }
        } else if (line.find("INSERT INTO game_sessions") != std::string::npos) {
            std::vector<std::string> values = parseValues(line);
            if (values.size() == 4) {
                allGameSessions.push_back(new GameSession(std::stoi(values[0]), values[1], values[2], values[3]));
            }
        } else if (line.find("INSERT INTO scores") != std::string::npos) {
            std::vector<std::string> values = parseValues(line);
            if (values.size() == 4) {
                allScores.push_back(new Score(std::stoi(values[0]), std::stoi(values[1]), std::stoi(values[2]), std::stoi(values[3])));
            }
        } else if (line.find("INSERT INTO rooms") != std::string::npos) {
            std::vector<std::string> values = parseValues(line);
            // The rooms table has description and ascii art, we only use description
            if (values.size() >= 2) {
                allRooms.push_back(new Room(values[1]));
            }
        }
    }
}

void Game::start() {
    printWelcomeMessage();
    render(); // Initial render
    gameLoop();
}

void Game::printWelcomeMessage() {
    std::cout << "Welcome to the Raycaster Game!" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
}

void Game::gameLoop() {
    char input = ' ';
    const double moveSpeed = 0.2;
    const double rotSpeed = 0.1;
    bool hasMoved = false;

    while (!gameOver) {
        std::cin >> input;
        hasMoved = false;

        if (input == 'q') {
            gameOver = true;
            continue;
        }

        // --- Movement ---
        if (input == 'w') {
            double newX = player->x + player->dirX * moveSpeed;
            double newY = player->y + player->dirY * moveSpeed;
            if (map->getTile(int(newX), int(player->y)) != '#') player->x = newX;
            if (map->getTile(int(player->x), int(newY)) != '#') player->y = newY;
            hasMoved = true;
        }
        if (input == 's') {
            double newX = player->x - player->dirX * moveSpeed;
            double newY = player->y - player->dirY * moveSpeed;
            if (map->getTile(int(newX), int(player->y)) != '#') player->x = newX;
            if (map->getTile(int(player->x), int(newY)) != '#') player->y = newY;
            hasMoved = true;
        }

        // --- Rotation ---
        if (input == 'd') { // Turn right
            double oldDirX = player->dirX;
            player->dirX = player->dirX * cos(-rotSpeed) - player->dirY * sin(-rotSpeed);
            player->dirY = oldDirX * sin(-rotSpeed) + player->dirY * cos(-rotSpeed);
            double oldPlaneX = player->planeX;
            player->planeX = player->planeX * cos(-rotSpeed) - player->planeY * sin(-rotSpeed);
            player->planeY = oldPlaneX * sin(-rotSpeed) + player->planeY * cos(-rotSpeed);
            hasMoved = true;
        }
        if (input == 'a') { // Turn left
            double oldDirX = player->dirX;
            player->dirX = player->dirX * cos(rotSpeed) - player->dirY * sin(rotSpeed);
            player->dirY = oldDirX * sin(rotSpeed) + player->dirY * cos(rotSpeed);
            double oldPlaneX = player->planeX;
            player->planeX = player->planeX * cos(rotSpeed) - player->planeY * sin(rotSpeed);
            player->planeY = oldPlaneX * sin(rotSpeed) + player->planeY * cos(rotSpeed);
            hasMoved = true;
        }

        if (hasMoved) {
            score++; // Increment score on any valid move/turn
            render();
        }
    }
    std::cout << "Game Over! Final Score: " << score << std::endl;
}

void Game::render() {
    // Clear console
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif

    std::vector<std::string> screen(screenHeight, std::string(screenWidth, ' '));
    const int panelWidth = 20;
    const int gameWidth = screenWidth - panelWidth;

    // --- Raycasting ---
    for (int x = 0; x < gameWidth; ++x) {
        double cameraX = 2 * x / double(gameWidth) - 1; // Recalculate cameraX for game view width
        double rayDirX = player->dirX + player->planeX * cameraX;
        double rayDirY = player->dirY + player->planeY * cameraX;

        int mapX = int(player->x);
        int mapY = int(player->y);

        double deltaDistX = (rayDirX == 0) ? 1e30 : std::abs(1 / rayDirX);
        double deltaDistY = (rayDirY == 0) ? 1e30 : std::abs(1 / rayDirY);

        double sideDistX, sideDistY;
        int stepX, stepY;

        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (player->x - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0 - player->x) * deltaDistX;
        }
        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (player->y - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0 - player->y) * deltaDistY;
        }

        int hit = 0;
        int side;
        while (hit == 0) {
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
            if (map->getTile(mapX, mapY) == '#') hit = 1;
        }

        double perpWallDist;
        if (side == 0) perpWallDist = (sideDistX - deltaDistX);
        else          perpWallDist = (sideDistY - deltaDistY);

        int lineHeight = (int)(screenHeight / perpWallDist);

        int drawStart = -lineHeight / 2 + screenHeight / 2;
        if (drawStart < 0) drawStart = 0;
        int drawEnd = lineHeight / 2 + screenHeight / 2;
        if (drawEnd >= screenHeight) drawEnd = screenHeight - 1;

        char wallChar;
        if (perpWallDist < 4) wallChar = '#';
        else if (perpWallDist < 8) wallChar = 'O';
        else if (perpWallDist < 14) wallChar = '=';
        else wallChar = '-';

        if (side == 1) {
            if (wallChar == '#') wallChar = '@';
            else if (wallChar == 'O') wallChar = 'o';
            else if (wallChar == '=') wallChar = '~';
        }

        for (int y = drawStart; y <= drawEnd; ++y) {
            screen[y][x] = wallChar;
        }

        for (int y = drawEnd + 1; y < screenHeight; ++y) {
            screen[y][x] = '.';
        }
    }

    // --- UI Side Panel ---
    int panelX = gameWidth + 2; // Starting X coordinate for panel text

    // Draw Score
    std::string scoreHeader = "SCORE";
    screen[1].replace(panelX, scoreHeader.length(), scoreHeader);
    std::string scoreValue = std::to_string(score);
    screen[2].replace(panelX, scoreValue.length(), scoreValue);

    // Draw Controls
    std::string controlsHeader = "CONTROLS";
    screen[4].replace(panelX, controlsHeader.length(), controlsHeader);
    screen[5].replace(panelX, 12, "W: Forward");
    screen[6].replace(panelX, 13, "S: Backward");
    screen[7].replace(panelX, 13, "A: Turn Left");
    screen[8].replace(panelX, 14, "D: Turn Right");
    screen[9].replace(panelX, 8, "Q: Quit");

    // --- Print Screen Buffer ---
    for (int y = 0; y < screenHeight; ++y) {
        std::cout << screen[y] << std::endl;
    }
}
