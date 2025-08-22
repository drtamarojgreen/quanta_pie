#include "Game.h"
#include "players/Player.h"
#include "Room.h"
#include "objects/Character.h"
#include "CSVParser.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <algorithm> // Required for std::transform
#include <cctype>    // Required for ::tolower
#include <conio.h>   // Required for _getch() and _kbhit() on Windows
#include <windows.h> // Required for Windows console API

Game::Game() : player(nullptr), gameOver(false), hConsole(GetStdHandle(STD_OUTPUT_HANDLE)), current_challenge(nullptr) {
    createWorld("sql/game_data.sql"); // This will be ignored now, but keeping for compatibility
}

Game::Game(const std::string& sql_file_path) : player(nullptr), gameOver(false), hConsole(GetStdHandle(STD_OUTPUT_HANDLE)), current_challenge(nullptr) {
    createWorld(sql_file_path); // This will be ignored now, but keeping for compatibility
}

// Destructor is now defaulted in Game.h due to unique_ptr usage

// Helper function to set cursor position
void SetCursorPosition(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Helper function to clear a specific region of the console
void ClearConsoleRegion(int x, int y, int width, int height) {
    DWORD count;
    COORD coord = { (SHORT)x, (SHORT)y };
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    GetConsoleScreenBufferInfo(hConsole, &csbi);

    // Fill the region with spaces
    FillConsoleOutputCharacter(hConsole, (TCHAR) ' ', width * height, coord, &count);

    // Fill the region with the current background color
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, width * height, coord, &count);

    // Set the cursor to the top-left of the region
    SetConsoleCursorPosition(hConsole, coord);
}

void Game::createWorld(const std::string& sql_file_path) {
    // Load all game data from the CSV files
    loadDataFromCSV();

    // Assign the first loaded player as the main player character.
    if (!allPlayers.empty()) {
        player = std::move(allPlayers[0]); // Transfer ownership
        // Assign the first loaded room as the starting location for the main player.
        if (!allRooms.empty()) {
            player->setCurrentRoom(allRooms[0].get()); // Pass raw pointer to Room
            // Example: Add a challenge to the starting room
            if (allRooms[0]->getChallenge() == nullptr) {
                std::vector<CBTChoice> choices;
                CBTChoice choice1;
                choice1.description = "Challenge the thought";
                choice1.action = [this]() { // Lambda captures 'this' (Game instance)
                    this->player->incrementScore(10);
                    this->current_challenge = nullptr; // Resolve challenge
                    // Add more complex outcomes here
                };
                choices.push_back(choice1);

                CBTChoice choice2;
                choice2.description = "Accept the thought";
                choice2.action = [this]() { // Lambda captures 'this' (Game instance)
                    this->player->incrementScore(-5);
                    this->current_challenge = nullptr; // Resolve challenge
                    // Add more complex outcomes here
                };
                choices.push_back(choice2);
                allRooms[0]->setChallenge(std::make_unique<Challenge>("You feel overwhelmed by the vastness of the void.", choices));
            }
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

std::vector<std::string> Game::getRoomInfoLines() {
    std::vector<std::string> lines;
    if (!player) return lines;

    if (current_challenge) {
        lines.push_back("----------------------------------------");
        lines.push_back("               CHALLENGE!               ");
        lines.push_back("----------------------------------------");
        lines.push_back("Thought: " + current_challenge->getThought());
        lines.push_back("");
        lines.push_back("Choices:");
        for (size_t i = 0; i < current_challenge->getChoices().size(); ++i) {
            lines.push_back(std::to_string(i + 1) + ". " + current_challenge->getChoices()[i].description);
        }
        lines.push_back("----------------------------------------");
    } else {
        Room* currentRoom = player->getCurrentRoom();
        lines.push_back(""); // Empty line for spacing
        lines.push_back(currentRoom->getDescription());

        const auto& characters = currentRoom->getCharacters();
        if (!characters.empty()) {
            lines.push_back(""); // Empty line for spacing
            for (const auto& character : characters) {
                lines.push_back(character->getDescription());
                lines.push_back("They say: \"" + character->getDialogue() + "\"");
            }
        }

        lines.push_back(""); // Empty line for spacing
        lines.push_back("It's you!");
        lines.push_back(player->getRepresentation());
        lines.push_back(""); // Empty line for spacing

        std::stringstream ss_exits;
        ss_exits << "Available exits:";
        for (auto const& [direction, room] : currentRoom->getAllExits()) { // Use getAllExits
            ss_exits << " " << direction;
        }
        lines.push_back(ss_exits.str());
    }
    return lines;
}

std::vector<std::string> Game::getSidePanelLines() {
    std::vector<std::string> lines;
    if (!player) return lines;

    Room* currentRoom = player->getCurrentRoom();

    lines.push_back("----------------------------------------");
    lines.push_back("               GAME INFO                ");
    lines.push_back("----------------------------------------");
    lines.push_back("Score: " + std::to_string(player->getScore()));
    lines.push_back("----------------------------------------");
    lines.push_back("               HELP                     ");
    lines.push_back("  - up/north, down/south, left/west, right/east to move.");
    lines.push_back("  - 'look': Look around.");
    lines.push_back("  - 'dance': Do a jig.");
    lines.push_back("  - 'help': Show commands.");
    lines.push_back("  - 'quit': Exit game.");
    lines.push_back("----------------------------------------");
    lines.push_back("               MAP                      ");
    lines.push_back("----------------------------------------");

    // Simple map representation
    std::map<std::string, Room*> exits = currentRoom->getAllExits();

    lines.push_back("       ");
    if (exits.count("north")) lines.back() += "[ ]"; else lines.back() += "   ";
    lines.push_back("       |");
    lines.push_back("       |");
    std::string middle_map_line = "";
    if (exits.count("west")) middle_map_line += "[ ]---"; else middle_map_line += "      ";
    middle_map_line += "[X]"; // Current room
    if (exits.count("east")) middle_map_line += "---[ ]";
    lines.push_back(middle_map_line);
    lines.push_back("       |");
    lines.push_back("       |");
    lines.push_back("       ");
    if (exits.count("south")) lines.back() += "[ ]"; else lines.back() += "   ";

    lines.push_back("----------------------------------------");
    return lines;
}

void Game::displayGameScreen() {
    // Clear screen using Windows API
    ClearConsoleRegion(0, 0, 120, 50); // Clear a large enough area

    std::vector<std::string> room_lines = getRoomInfoLines();
    std::vector<std::string> side_panel_lines = getSidePanelLines();

    // Determine max height
    size_t max_height = std::max(room_lines.size(), side_panel_lines.size());

    const int GAME_AREA_WIDTH = 60; // Adjust as needed
    const int SIDE_PANEL_START_X = GAME_AREA_WIDTH + 2; // 2 spaces between game area and side panel

    for (size_t i = 0; i < max_height; ++i) {
        std::string room_line = (i < room_lines.size()) ? room_lines[i] : "";
        std::string side_panel_line = (i < side_panel_lines.size()) ? side_panel_lines[i] : "";

        // Print room line
        SetCursorPosition(0, i);
        std::cout << room_line;

        // Print side panel line
        SetCursorPosition(SIDE_PANEL_START_X, i);
        std::cout << side_panel_line;
    }
    // Set cursor position for input prompt
    SetCursorPosition(0, max_height + 1);
}

void Game::printWelcomeMessage() {
    std::cout << "Welcome to Quanta_Pie!" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    displayGameScreen(); // Use the new display function
    // printHelp(); // Help is now part of the side panel
}

void Game::gameLoop() {
    while (!gameOver) {
        if (g_signal_received) { // Check for signal
            gameOver = true;
            continue;
        }
        displayGameScreen(); // Display combined screen at the beginning of each loop
        std::cout << "> ";

        int ch = _getch(); // Read a single character

        // Handle extended keys (like arrow keys)
        if (ch == 0 || ch == 0xE0) {
            ch = _getch(); // Read the second byte for extended key
            switch (ch) {
                case 72: // Up arrow
                    processInput("north");
                    break;
                case 80: // Down arrow
                    processInput("south");
                    break;
                case 75: // Left arrow
                    processInput("west");
                    break;
                case 77: // Right arrow
                    processInput("east");
                    break;
                default:
                    // Ignore other extended keys
                    break;
            }
        } else {
            // Handle regular characters
            std::string input_str(1, static_cast<char>(ch)); // Convert char to string
            if (input_str == "q") { // 'q' for quit
                gameOver = true;
                continue;
            }
            // If a challenge is active, process input as a choice number
            else if (current_challenge) {
                int choice_num = -1;
                try {
                    choice_num = std::stoi(input_str);
                } catch (const std::invalid_argument& e) {
                    // Not a number, ignore or provide feedback
                } catch (const std::out_of_range& e) {
                    // Number too large/small, ignore or provide feedback
                }

                if (choice_num > 0 && choice_num <= current_challenge->getChoices().size()) {
                    current_challenge->getChoices()[choice_num - 1].action(); // Execute the chosen action
                } else {
                    // Invalid choice for challenge, ignore or provide feedback
                }
            }
            else if (input_str == "h") { // 'h' for help
                processInput("help");
            } else if (input_str == "l") { // 'l' for look
                processInput("look");
            } else if (input_str == "d") { // 'd' for dance
                processInput("dance");
            } else {
                // For other single character inputs, pass them to processInput
                // This allows for future single-character commands
                processInput(input_str);
            }
        }
    }
    std::cout << "Thank you for playing Quanta_Pie!" << std::endl;
}

void Game::processInput(const std::string& input) {
    if (!player) return;

    std::string lowerInput = input;
    // Convert input to lowercase for case-insensitive comparison
    std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);

    // If a challenge is active, process input as a choice number
    if (current_challenge) {
        // This part is now handled in gameLoop directly for single-key input
        // This function will only be called by gameLoop for mapped commands (north, south, etc.)
        return;
    }

    // Process normal game commands
    if (lowerInput == "help") {
        // Help is now part of the side panel, no separate print needed here
        // Or, if a specific help message is desired, it should be handled differently
    } else if (lowerInput == "look") {
        // Look just updates the display, which happens automatically in gameLoop
    } else if (lowerInput == "dance") {
        // This is a temporary message, will be replaced by combat/action messages
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
        Room* nextRoom = current->getExit(lowerInput);

        if (nextRoom != nullptr) {
            player->setCurrentRoom(nextRoom);
            player->incrementScore(); // Increment score on successful move
            // Check for challenge in the new room
            if (nextRoom->getChallenge() != nullptr) {
                current_challenge = std::make_unique<Challenge>(nextRoom->getChallenge()->getThought(), nextRoom->getChallenge()->getChoices());
            }
        } else {
            // This message will be overwritten by the next displayGameScreen() call
            // Consider a temporary message area or a more robust message system
            std::cout << "You can't go that way. Type 'help' for a list of commands." << std::endl;
        }
    }
}