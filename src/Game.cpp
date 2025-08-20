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
    // Load room data from the SQL file
    loadDataFromSQL("sql/game_data.sql");

    // The current implementation of loadDataFromSQL does not allow for linking rooms
    // by name or ID. The old linking logic is left here as a reference, but it will
    // not work as is, because the pointers (library, archives, readingNook) no longer exist.
    // This is in accordance with the "Gated Direct-to-Publish Protocol".

    // Link the rooms (This part will not work without specific room pointers)
    // if (allRooms.size() >= 3) {
    //     allRooms[0]->addExit("north", allRooms[1]);
    //     allRooms[0]->addExit("east", allRooms[2]);
    //     allRooms[1]->addExit("south", allRooms[0]);
    //     allRooms[2]->addExit("west", allRooms[0]);
    // }

    // Create the player and set the starting room.
    // The player will start in the first room loaded from the SQL file.
    if (!allRooms.empty()) {
        player = new Player(allRooms[0]);
    } else {
        // As a fallback, create a default room if the SQL file is empty or fails to load
        Room* defaultRoom = new Room("You are in an empty, non-descript void. Something went wrong with the world creation.");
        allRooms.push_back(defaultRoom);
        player = new Player(defaultRoom);
    }
}

void Game::loadDataFromSQL(const std::string& filename) {
    std::ifstream sqlFile(filename);
    if (!sqlFile.is_open()) {
        // Since this is a text-based game, we can report errors to std::cerr.
        // In a real application, you might use a more robust logging system.
        // std::cerr << "Error: Could not open SQL file " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(sqlFile, line)) {
        // A simple check for the INSERT statement for rooms
        if (line.find("INSERT INTO rooms") != std::string::npos) {
            // This is a very fragile parser. It assumes the format:
            // INSERT INTO rooms (room_id, description, ascii_art) VALUES (1, 'description text', 'ascii art');

            // Find the description between the first and second single quotes
            size_t firstQuote = line.find('\'');
            if (firstQuote == std::string::npos) continue;
            size_t secondQuote = line.find('\'', firstQuote + 1);
            if (secondQuote == std::string::npos) continue;

            std::string description = line.substr(firstQuote + 1, secondQuote - firstQuote - 1);

            // The Room class currently only supports a description.
            // The ascii_art is parsed but not used, to adhere to the protocol
            // of not modifying unrelated files (like Room.h).
            Room* newRoom = new Room(description);
            allRooms.push_back(newRoom);
        }
    }
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
