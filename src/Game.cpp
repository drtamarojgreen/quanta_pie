#include "Game.h"
#include "players/Player.h"
#include "Room.h"
#include "objects/Character.h"
#include "objects/Challenge.h"
#include "GameSession.h"
#include "Score.h"
#include "CSVParser.h"
#include "platform/WindowsConsole.h" // Include the Windows-specific console implementation
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <algorithm> // Required for std::transform
#include <cctype>    // Required for ::tolower
Game::Game() : console(std::make_unique<WindowsConsole>()), player(nullptr), gameOver(false), current_challenge(nullptr) {
    createWorld("sql/game_data.sql"); // This will be ignored now, but keeping for compatibility
}

Game::Game(const std::string& sql_file_path) : console(std::make_unique<WindowsConsole>()), player(nullptr), gameOver(false), current_challenge(nullptr) {
    createWorld(sql_file_path); // This will be ignored now, but keeping for compatibility
}

Game::~Game() = default; // Explicitly defaulted in .cpp file

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
            auto newCharacter = std::make_unique<Character>(std::stoi(characterData[i][0]), name, description, initialRoomId, dialogue);
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

    // Load Tools
    std::vector<std::vector<std::string>> toolData = CSVParser::readCSV("sql/tools.csv");
    std::cout << "Loading Tools..." << std::endl;
    for (size_t i = 1; i < toolData.size(); ++i) { // Skip header row
        if (toolData[i].size() > 3) {
            allTools.push_back(std::make_unique<Tool>(std::stoi(toolData[i][0]), toolData[i][1], toolData[i][2], std::stoi(toolData[i][3])));
        } else {
            std::cerr << "Error: Malformed tool data at row " << i << std::endl;
        }
    }

    // Load RoomObjects
    std::vector<std::vector<std::string>> roomObjectData = CSVParser::readCSV("sql/room_objects.csv");
    std::cout << "Loading RoomObjects..." << std::endl;
    for (size_t i = 1; i < roomObjectData.size(); ++i) { // Skip header row
        if (roomObjectData[i].size() > 3) {
            allRoomObjects.push_back(std::make_unique<RoomObject>(std::stoi(roomObjectData[i][0]), roomObjectData[i][1], roomObjectData[i][2], std::stoi(roomObjectData[i][3])));
        } else {
            std::cerr << "Error: Malformed room object data at row " << i << std::endl;
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
        lines.push_back("A sudden thought crosses your mind, demanding a response.");
        lines.push_back("Thought: " + current_challenge->getThought());
        lines.push_back("");
        lines.push_back("How do you respond? (Enter the number of your choice)");
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
    lines.push_back("            COMMANDS                    ");
    lines.push_back("  - To move, type a direction:");
    lines.push_back("    'north', 'south', 'east', 'west'");
    lines.push_back("  - Other commands:");
    lines.push_back("    'look', 'dance', 'quit'");
    lines.push_back("----------------------------------------");
    lines.push_back("               MAP                      ");
    lines.push_back("----------------------------------------");

    // Simple map representation
    std::map<std::string, Room*> exits = currentRoom->getAllExits();

    lines.push_back("       ");
    if (exits.count("north")) lines.back() += "[N]"; else lines.back() += "   ";
    lines.push_back("       |");
    lines.push_back("       |");
    std::string middle_map_line = "";
    if (exits.count("west")) middle_map_line += "[W]---"; else middle_map_line += "      ";
    middle_map_line += "[X]"; // Current room
    if (exits.count("east")) middle_map_line += "---[E]";
    lines.push_back(middle_map_line);
    lines.push_back("       |");
    lines.push_back("       |");
    lines.push_back("       ");
    if (exits.count("south")) lines.back() += "[S]"; else lines.back() += "   ";

    lines.push_back("----------------------------------------");
    return lines;
}

void Game::displayGameScreen() {
    // Clear screen using the console abstraction
    console->clear();

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
        console->setCursorPosition(0, i);
        std::cout << room_line;

        // Print side panel line
        console->setCursorPosition(SIDE_PANEL_START_X, i);
        std::cout << side_panel_line;
    }
    // Set cursor position for input prompt
    console->setCursorPosition(0, max_height + 1);
}

void Game::printWelcomeMessage() {
    console->clear(); // Clear screen before showing the message
    std::cout << "Welcome to Quanta_Pie!" << std::endl;
    std::cout << std::endl;
    std::cout << "This is a game of exploration and introspection." << std::endl;
    std::cout << "You find yourself in a strange, shifting world. Your goal is to navigate" << std::endl;
    std::cout << "the rooms, interact with characters, and face the challenges you encounter." << std::endl;
    std::cout << std::endl;
    std::cout << "Type commands and press Enter to interact with the world." << std::endl;
    std::cout << "Basic commands are listed in the HELP panel on the right." << std::endl;
    std::cout << std::endl;
    std::cout << "Press Enter to begin..." << std::endl;
    std::string dummy;
    std::getline(std::cin, dummy); // Wait for user to press Enter

    displayGameScreen(); // Initial display
}

void Game::gameLoop() {
    while (!gameOver) {
        if (g_signal_received) { // Check for signal
            gameOver = true;
            continue;
        }

        displayGameScreen(); // Refresh screen at the start of every turn

        std::cout << "> ";
        std::string input_line;
        if (!std::getline(std::cin, input_line)) {
            // Handle EOF (Ctrl+D on Unix, Ctrl+Z on Windows)
            if (std::cin.eof()) {
                gameOver = true;
                std::cout << std::endl << "Exiting game due to end-of-file." << std::endl;
                continue;
            }
        }

        // Convert input to lowercase for case-insensitive comparison
        std::transform(input_line.begin(), input_line.end(), input_line.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        if (input_line == "quit") {
            gameOver = true;
        } else {
            processInput(input_line);
        }
    }
    std::cout << "Thank you for playing Quanta_Pie!" << std::endl;
}

void Game::processInput(const std::string& input) {
    if (!player) return;

    // Input is already lowercased in gameLoop
    const std::string& lowerInput = input;

    // If a challenge is active, process input as a choice number
    if (current_challenge) {
        int choice_num = -1;
        try {
            // Attempt to convert the entire input string to a number
            choice_num = std::stoi(lowerInput);
        } catch (const std::invalid_argument& e) {
            // Not a number, maybe it's a different command?
            // For now, we'll just say it's an invalid choice.
            // A more complex implementation could allow other commands during a challenge.
        } catch (const std::out_of_range& e) {
            // Number too large/small
        }

        if (choice_num > 0 && static_cast<size_t>(choice_num) <= current_challenge->getChoices().size()) {
            // Execute the chosen action. The lambda will set current_challenge to nullptr.
            current_challenge->getChoices()[choice_num - 1].action();
        } else {
            // Invalid choice. We can add a message to the player here.
            // For now, doing nothing is fine, the screen will just refresh.
        }
        return; // Stop further processing after handling challenge input
    }

    // Process normal game commands
    if (lowerInput == "help") {
        // The help panel is always visible. This command is a no-op.
    } else if (lowerInput == "look") {
        // 'look' simply forces a screen refresh, which happens on every loop.
    } else if (lowerInput == "dance") {
        // This is a temporary message, will be replaced by combat/action messages
        std::cout << "You do a little jig. It's surprisingly uplifting." << std::endl;
    } else {
        // Any other command is assumed to be a move attempt.
        Room* current = player->getCurrentRoom();
        Room* nextRoom = current->getExit(lowerInput);

        if (nextRoom != nullptr) {
            player->setCurrentRoom(nextRoom);
            player->incrementScore(); // Increment score on successful move
            // Check for challenge in the new room
            if (nextRoom->getChallenge() != nullptr) {
                current_challenge = std::make_unique<Challenge>(*nextRoom->getChallenge());
            }
        } else {
            // We can add a message to a message buffer to be displayed on the next screen refresh
            // For now, we'll just let the screen refresh, which shows the command was ineffective.
        }
    }
}