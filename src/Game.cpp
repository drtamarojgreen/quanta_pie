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
