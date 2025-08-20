#include "Game.h"
#include "Player.h"
#include "Room.h"
#include <iostream>

Game::Game() : player(nullptr), gameOver(false) {
    createWorld();
    // The player will be initialized in createWorld
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
