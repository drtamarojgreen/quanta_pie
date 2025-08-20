#include "Game.h"
#include "Player.h"
#include "Room.h"
#include <iostream>

Game::Game() : player(nullptr), gameOver(false) {
    createWorld();
    // The player will be initialized in createWorld
}

Game::~Game() {
    delete player;
    for (auto room : allRooms) {
        delete room;
    }
}

void Game::createWorld() {
    // Create the rooms
    Room* library = new Room("You find yourself in a vast, circular library, its towering shelves carved from a single, colossal petrified tree. The air hums with a faint, magical resonance, and the only light filters down from glowing crystals embedded in the ceiling, illuminating a massive, rune-etched obsidian desk at the room's heart.");
    Room* archives = new Room("The air is thick with the scent of ancient paper and preservation wards. You are in the archives, a labyrinth of impossibly tall shelves that disappear into the gloom above. Each shelf is crammed with scrolls, codices, and leather-bound tomes, their spines either blank or marked with cryptic symbols that seem to shift when you're not looking directly at them.");
    Room* readingNook = new Room("Tucked away behind a tapestry depicting a forgotten battle, you discover a hidden nook. A plush, high-backed armchair sits before a fireplace where the flames burn a soothing, ethereal blue. A small, floating orb of light provides perfect illumination for reading, and the gentle crackling of the fire is the only sound.");

    // Add rooms to the game's list to manage memory
    allRooms.push_back(library);
    allRooms.push_back(archives);
    allRooms.push_back(readingNook);

    // Link the rooms
    library->addExit("north", archives);
    library->addExit("east", readingNook);

    archives->addExit("south", library);

    readingNook->addExit("west", library);

    // Create the player and set the starting room
    player = new Player(library);
}

void Game::start() {
    printWelcomeMessage();
    gameLoop();
}

void Game::printWelcomeMessage() {
    std::cout << "Welcome to Quanta_Pie!" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    if (player) {
        std::cout << player->getCurrentRoom()->getDescription() << std::endl;
        std::cout << std::endl << "It's you!" << std::endl;
        std::cout << player->getRepresentation() << std::endl << std::endl;
        player->getCurrentRoom()->printExits();
    }
    printHelp();
}

void Game::printHelp() {
    std::cout << std::endl;
    std::cout << "--- Game Commands ---" << std::endl;
    std::cout << "  - [direction]: Type 'north', 'south', 'east', or 'west' to move." << std::endl;
    std::cout << "  - 'look':         Look around the room again." << std::endl;
    std::cout << "  - 'dance':        Do a little dance." << std::endl;
    std::cout << "  - 'help':         Show this list of commands." << std::endl;
    std::cout << "  - 'quit':         Exit the game." << std::endl;
    std::cout << "---------------------" << std::endl;
    std::cout << std::endl;
}

void Game::gameLoop() {
    char input = ' ';
    const double moveSpeed = 0.2;
    const double rotSpeed = 0.1;

    while (!gameOver) {
        std::cin >> input;

        if (input == 'q') {
            gameOver = true;
            continue;
        }

        // --- Movement ---
        if (input == 'w') {
            double newX = player->x + player->dirX * moveSpeed;
            double newY = player->y + player->dirY * moveSpeed;
            // Collision detection
            if (map->getTile(int(newX), int(player->y)) != '#') player->x = newX;
            if (map->getTile(int(player->x), int(newY)) != '#') player->y = newY;
        }
        if (input == 's') {
            double newX = player->x - player->dirX * moveSpeed;
            double newY = player->y - player->dirY * moveSpeed;
            // Collision detection
            if (map->getTile(int(newX), int(player->y)) != '#') player->x = newX;
            if (map->getTile(int(player->x), int(newY)) != '#') player->y = newY;
        }

        // --- Rotation ---
        if (input == 'd') { // Turn right
            double oldDirX = player->dirX;
            player->dirX = player->dirX * cos(-rotSpeed) - player->dirY * sin(-rotSpeed);
            player->dirY = oldDirX * sin(-rotSpeed) + player->dirY * cos(-rotSpeed);
            double oldPlaneX = player->planeX;
            player->planeX = player->planeX * cos(-rotSpeed) - player->planeY * sin(-rotSpeed);
            player->planeY = oldPlaneX * sin(-rotSpeed) + player->planeY * cos(-rotSpeed);
        }
        if (input == 'a') { // Turn left
            double oldDirX = player->dirX;
            player->dirX = player->dirX * cos(rotSpeed) - player->dirY * sin(rotSpeed);
            player->dirY = oldDirX * sin(rotSpeed) + player->dirY * cos(rotSpeed);
            double oldPlaneX = player->planeX;
            player->planeX = player->planeX * cos(rotSpeed) - player->planeY * sin(rotSpeed);
            player->planeY = oldPlaneX * sin(rotSpeed) + player->planeY * cos(rotSpeed);
        }

        if (input == 'w' || input == 's' || input == 'a' || 'd') {
            render();
            std::cout << "Controls: W (forward), S (backward), A (turn left), D (turn right), Q (quit)" << std::endl;
        }
    }
    std::cout << "Game Over!" << std::endl;
}

void Game::render() {
    // Clear console and print the screen buffer
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif

    std::vector<std::string> screen(screenHeight, std::string(screenWidth, ' '));

    for (int x = 0; x < screenWidth; ++x) {
        double cameraX = 2 * x / double(screenWidth) - 1;
        double rayDirX = player->dirX + player->planeX * cameraX;
        double rayDirY = player->dirY + player->planeY * cameraX;

        int mapX = int(player->x);
        int mapY = int(player->y);

        double deltaDistX = (rayDirX == 0) ? 1e30 : std::abs(1 / rayDirX);
        double deltaDistY = (rayDirY == 0) ? 1e30 : std::abs(1 / rayDirY);

        double sideDistX;
        double sideDistY;

        int stepX;
        int stepY;

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
        if (side == 0) {
            perpWallDist = (sideDistX - deltaDistX);
        } else {
            perpWallDist = (sideDistY - deltaDistY);
        }

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

    for (int y = 0; y < screenHeight; ++y) {
        std::cout << screen[y] << std::endl;
    }
}
