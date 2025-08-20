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
    Room* library = new Room("You are in a dimly lit library. A grand oak desk sits in the center.");
    Room* archives = new Room("You are in a dusty archive room. Shelves packed with old scrolls line the walls.");
    Room* readingNook = new Room("You've entered a cozy reading nook with a crackling fireplace.");

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
    std::cout << "Type 'quit' to exit the game." << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    if (player) {
        std::cout << player->getCurrentRoom()->getDescription() << std::endl;
        player->getCurrentRoom()->printExits();
    }
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

    if (input == "look") {
        std::cout << player->getCurrentRoom()->getDescription() << std::endl;
        player->getCurrentRoom()->printExits();
    } else if (input == "inventory" || input == "i") {
        const auto& inventory = player->getInventory();
        if (inventory.empty()) {
            std::cout << "You are not carrying anything." << std::endl;
        } else {
            std::cout << "You are carrying:" << std::endl;
            for (const auto& item : inventory) {
                std::cout << " - " << item << std::endl;
            }
        }
    } else {
        Room* current = player->getCurrentRoom();
        Room* nextRoom = current->getExit(input);

        if (nextRoom != nullptr) {
            player->setCurrentRoom(nextRoom);
            std::cout << std::endl;
            std::cout << player->getCurrentRoom()->getDescription() << std::endl;
            player->getCurrentRoom()->printExits();
        } else {
            std::cout << "You can't go that way." << std::endl;
        }
    }
}
