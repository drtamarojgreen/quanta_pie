#include "Game.h"
#include "Player.h"
#include "Room.h"
#include "Character.h"
#include "CSVParser.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <algorithm> // Required for std::transform
#include <cctype>    // Required for ::tolower

Game::Game() : player(nullptr), gameOver(false) {
    createWorld("sql/game_data.sql"); // This will be ignored now, but keeping for compatibility
}

Game::Game(const std::string& sql_file_path) : player(nullptr), gameOver(false) {
    createWorld(sql_file_path); // This will be ignored now, but keeping for compatibility
}

// Destructor is now defaulted in Game.h due to unique_ptr usage

void Game::createWorld(const std::string& sql_file_path) {
    // Load all game data from the CSV files
    loadDataFromCSV();

    // Assign the first loaded player as the main player character.
    if (!allPlayers.empty()) {
        player = std::move(allPlayers[0]); // Transfer ownership
        // Assign the first loaded room as the starting location for the main player.
        if (!allRooms.empty()) {
            player->setCurrentRoom(allRooms[0].get()); // Pass raw pointer to Room
        }
    } else {
        // Fallback: if no players or rooms are loaded, create defaults.
        if (allRooms.empty()) {
            allRooms.push_back(std::make_unique<Room>("A non-descript, empty void."));
        }
        player = std::make_unique<Player>(0, "Default Player", "unknown", allRooms[0].get());
        allPlayers.push_back(std::move(player)); // Add to allPlayers, then move back to player
        player = std::move(allPlayers[0]);
    }
}

void Game::loadDataFromCSV() {
    // Load Rooms
    std::vector<std::vector<std::string>> roomData = CSVParser::readCSV("sql/rooms.csv");
    std::cout << "Loading Rooms..." << std::endl;
    for (size_t i = 1; i < roomData.size(); ++i) { // Skip header row
        if (roomData[i].size() > 1) {
            std::cout << "Room Description: " << roomData[i][1] << std::endl;
            allRooms.push_back(std::make_unique<Room>(roomData[i][1]));
        } else {
            std::cerr << "Error: Malformed room data at row " << i << std::endl;
        }
    }

    // Load Characters
    std::vector<std::vector<std::string>> characterData = CSVParser::readCSV("sql/characters.csv");
    std::cout << "Loading Characters..." << std::endl;
    for (size_t i = 1; i < characterData.size(); ++i) { // Skip header row
        if (characterData[i].size() > 4) {
            std::string name = characterData[i][1];
            std::string description = characterData[i][2];
            int initialRoomId = std::stoi(characterData[i][3]);
            std::string dialogue = characterData[i][4];
            std::cout << "Character Name: " << name << ", Description: " << description << ", Dialogue: " << dialogue << std::endl;
            auto newCharacter = std::make_unique<Character>(name, description, dialogue);
            if (initialRoomId > 0 && initialRoomId <= allRooms.size()) {
                allRooms[initialRoomId - 1]->addCharacter(newCharacter.get()); // Pass raw pointer to Room
            }
            allCharacters.push_back(std::move(newCharacter));
        } else {
            std::cerr << "Error: Malformed character data at row " << i << std::endl;
        }
    }

    // Load Players
    std::vector<std::vector<std::string>> playerData = CSVParser::readCSV("sql/players.csv");
    std::cout << "Loading Players..." << std::endl;
    for (size_t i = 1; i < playerData.size(); ++i) { // Skip header row
        if (playerData[i].size() > 2) {
            allPlayers.push_back(std::make_unique<Player>(std::stoi(playerData[i][0]), playerData[i][1], playerData[i][2], nullptr));
        } else {
            std::cerr << "Error: Malformed player data at row " << i << std::endl;
        }
    }

    // Load Game Sessions
    std::vector<std::vector<std::string>> gameSessionData = CSVParser::readCSV("sql/game_sessions.csv");
    std::cout << "Loading Game Sessions..." << std::endl;
    for (size_t i = 1; i < gameSessionData.size(); ++i) { // Skip header row
        if (gameSessionData[i].size() > 3) {
            allGameSessions.push_back(std::make_unique<GameSession>(std::stoi(gameSessionData[i][0]), gameSessionData[i][1], gameSessionData[i][2], gameSessionData[i][3]));
        } else {
            std::cerr << "Error: Malformed game session data at row " << i << std::endl;
        }
    }

    // Load Scores
    std::vector<std::vector<std::string>> scoreData = CSVParser::readCSV("sql/scores.csv");
    std::cout << "Loading Scores..." << std::endl;
    for (size_t i = 1; i < scoreData.size(); ++i) { // Skip header row
        if (scoreData[i].size() > 3) {
            allScores.push_back(std::make_unique<Score>(std::stoi(scoreData[i][0]), std::stoi(scoreData[i][1]), std::stoi(scoreData[i][2]), std::stoi(scoreData[i][3])));
        } else {
            std::cerr << "Error: Malformed score data at row " << i << std::endl;
        }
    }

    // Load Exits (after all rooms are loaded)
    std::vector<std::vector<std::string>> exitData = CSVParser::readCSV("sql/exits.csv");
    std::cout << "Loading Exits..." << std::endl;
    for (size_t i = 1; i < exitData.size(); ++i) { // Skip header row
        if (exitData[i].size() > 3) {
            int fromRoomId = std::stoi(exitData[i][1]);
            int toRoomId = std::stoi(exitData[i][2]);
            std::string direction = exitData[i][3];

            if (fromRoomId > 0 && fromRoomId <= allRooms.size() &&
                toRoomId > 0 && toRoomId <= allRooms.size()) {
                allRooms[fromRoomId - 1]->addExit(direction, allRooms[toRoomId - 1].get());
            } else {
                std::cerr << "Error: Invalid room ID in exit data at row " << i << std::endl;
            }
        } else {
            std::cerr << "Error: Malformed exit data at row " << i << std::endl;
        }
    }
}

void Game::start() {
    printWelcomeMessage();
    gameLoop();
}

void Game::printCurrentRoomInfo() {
    if (!player) return;

    Room* currentRoom = player->getCurrentRoom();
    std::cout << std::endl;
    std::cout << currentRoom->getDescription() << std::endl;

    const auto& characters = currentRoom->getCharacters();
    if (!characters.empty()) {
        std::cout << std::endl;
        for (const auto& character : characters) {
            std::cout << character->getDescription() << std::endl;
            std::cout << "They say: \"" << character->getDialogue() << "\"" << std::endl;
        }
    }

    std::cout << std::endl << "It's you!" << std::endl;
    std::cout << player->getRepresentation() << std::endl << std::endl;
    currentRoom->printExits();
}

void Game::printWelcomeMessage() {
    std::cout << "Welcome to Quanta_Pie!" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    printCurrentRoomInfo();
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

void Game::printSidePanel() {
    if (!player) return;

    Room* currentRoom = player->getCurrentRoom();

    // Clear screen (platform-specific, for simplicity, just print newlines)
    // For a true clear screen, platform-specific calls like system("cls") or system("clear") would be needed,
    // but are generally avoided in portable C++ applications.
    for (int i = 0; i < 50; ++i) {
        std::cout << std::endl;
    }

    std::cout << "----------------------------------------" << std::endl;
    std::cout << "               GAME INFO                " << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Score: " << player->getScore() << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "               HELP                     " << std::endl;
    std::cout << "  - up/north, down/south, left/west, right/east to move." << std::endl;
    std::cout << "  - 'look': Look around." << std::endl;
    std::cout << "  - 'dance': Do a jig." << std::endl;
    std::cout << "  - 'help': Show commands." << std::endl;
    std::cout << "  - 'quit': Exit game." << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "               MAP                      " << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // Simple map representation
    // This assumes a grid-like layout, which might not be true for all room connections.
    // For a more accurate map, a graph traversal and rendering would be needed.
    // For now, just show current room and direct exits.
    std::map<std::string, Room*> exits = currentRoom->getAllExits();

    std::cout << "       ";
    if (exits.count("north")) std::cout << "[ ]"; else std::cout << "   ";
    std::cout << std::endl;
    std::cout << "       |" << std::endl;
    std::cout << "       |" << std::endl;
    if (exits.count("west")) std::cout << "[ ]---"; else std::cout << "      ";
    std::cout << "[X]"; // Current room
    if (exits.count("east")) std::cout << "---[ ]";
    std::cout << std::endl;
    std::cout << "       |" << std::endl;
    std::cout << "       |" << std::endl;
    std::cout << "       ";
    if (exits.count("south")) std::cout << "[ ]"; else std::cout << "   ";
    std::cout << std::endl;

    std::cout << "----------------------------------------" << std::endl;
}

void Game::gameLoop() {
    std::string input;
    while (!gameOver) {
        printSidePanel(); // Display side panel at the beginning of each loop
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

    std::string lowerInput = input;
    // Convert input to lowercase for case-insensitive comparison
    std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);

    if (lowerInput == "help") {
        printHelp();
    } else if (lowerInput == "look") {
        printCurrentRoomInfo();
    } else if (lowerInput == "dance") {
        std::cout << "You do a little jig. It's surprisingly uplifting." << std::endl;
    } else if (lowerInput == "up") {
        processInput("north"); // Map 'up' to 'north'
    } else if (lowerInput == "down") {
        processInput("south"); // Map 'down' to 'south'
    } else if (lowerInput == "left") {
        processInput("west"); // Map 'left' to 'west'
    } else if (lowerInput == "right") {
        processInput("east"); // Map 'right' to 'east'
    }
    else {
        // Try to move
        Room* current = player->getCurrentRoom();
        Room* nextRoom = current->getExit(lowerInput); // Use lowerInput for direction

        if (nextRoom != nullptr) {
            player->setCurrentRoom(nextRoom);
            player->incrementScore(); // Increment score on successful move
            printCurrentRoomInfo();
        } else {
            std::cout << "You can't go that way. Type 'help' for a list of commands." << std::endl;
        }
    }
}