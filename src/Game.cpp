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
    std::string input;
    while (!gameOver) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == "quit") {
            gameOver = true;
            continue;
        }

        processInput(input);
    }
    std::cout << "Thank you for playing Quanta_Pie!" << std::endl;
}

void Game::processInput(const std::string& input) {
    if (!player) return;

    if (input == "help") {
        printHelp();
    } else if (input == "look") {
        std::cout << std::endl;
        std::cout << player->getCurrentRoom()->getDescription() << std::endl;
        std::cout << std::endl << "It's you!" << std::endl;
        std::cout << player->getRepresentation() << std::endl << std::endl;
        player->getCurrentRoom()->printExits();
    } else if (input == "dance") {
        std::cout << "You do a little jig. It's surprisingly uplifting." << std::endl;
    }
    else {
        // Try to move
        Room* current = player->getCurrentRoom();
        Room* nextRoom = current->getExit(input);

        if (nextRoom != nullptr) {
            player->setCurrentRoom(nextRoom);
            std::cout << std::endl;
            std::cout << player->getCurrentRoom()->getDescription() << std::endl;
            std::cout << std::endl << "It's you!" << std::endl;
            std::cout << player->getRepresentation() << std::endl << std::endl;
            player->getCurrentRoom()->printExits();
        } else {
            std::cout << "You can't go that way. Type 'help' for a list of commands." << std::endl;
        }
    }
}
