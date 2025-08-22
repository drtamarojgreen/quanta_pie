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

    // Load Tools
    std::vector<std::vector<std::string>> toolData = CSVParser::readCSV("sql/tools.csv");
    std::cout << "Loading Tools..." << std::endl;
    for (size_t i = 1; i < toolData.size(); ++i) { // Skip header row
        if (toolData[i].size() > 3) {
            int toolId = std::stoi(toolData[i][0]);
            std::string name = toolData[i][1];
            std::string description = toolData[i][2];
            int initialRoomId = std::stoi(toolData[i][3]);

            auto newTool = std::make_unique<Tool>(toolId, name, description, initialRoomId);

            if (initialRoomId > 0 && static_cast<size_t>(initialRoomId) <= allRooms.size()) {
                allRooms[initialRoomId - 1]->addTool(newTool.get());
            } else {
                std::cerr << "Error: Invalid room ID " << initialRoomId << " for tool '" << name << "' at row " << i << std::endl;
            }
            allTools.push_back(std::move(newTool));
        } else {
            std::cerr << "Error: Malformed tool data at row " << i << std::endl;
        }
    }

    // Load RoomObjects
    std::vector<std::vector<std::string>> roomObjectData = CSVParser::readCSV("sql/room_objects.csv");
    std::cout << "Loading RoomObjects..." << std::endl;
    for (size_t i = 1; i < roomObjectData.size(); ++i) { // Skip header row
        if (roomObjectData[i].size() > 3) {
            int objectId = std::stoi(roomObjectData[i][0]);
            std::string name = roomObjectData[i][1];
            std::string description = roomObjectData[i][2];
            int roomId = std::stoi(roomObjectData[i][3]);

            auto newRoomObject = std::make_unique<RoomObject>(objectId, name, description, roomId);

            if (roomId > 0 && static_cast<size_t>(roomId) <= allRooms.size()) {
                allRooms[roomId - 1]->addObject(newRoomObject.get());
            } else {
                std::cerr << "Error: Invalid room ID " << roomId << " for room object '" << name << "' at row " << i << std::endl;
            }
            allRoomObjects.push_back(std::move(newRoomObject));
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

        const auto& objects = currentRoom->getObjects();
        if (!objects.empty()) {
            lines.push_back(""); // Empty line for spacing
            lines.push_back("You notice:");
            for (const auto& object : objects) {
                lines.push_back(" - " + object->getName());
            }
        }

        const auto& tools = currentRoom->getTools();
        if (!tools.empty()) {
            lines.push_back(""); // Empty line for spacing
            lines.push_back("On the floor, you see:");
            for (const auto& tool : tools) {
                lines.push_back(" - A " + tool->getName());
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
    lines.push_back("             INVENTORY                ");
    const auto& inventory = player->getTools();
    if (inventory.empty()) {
        lines.push_back("  (empty)");
    } else {
        for (const auto& tool : inventory) {
            lines.push_back("  - " + tool->getName());
        }
    }
    lines.push_back("----------------------------------------");
    lines.push_back("               HELP                     ");
    lines.push_back("  - move: n, s, e, w, up, down, etc.");
    lines.push_back("  - look:               (see room)");
    lines.push_back("  - look at <object>:   (describe)");
    lines.push_back("  - take <tool>:        (pick up)");
    lines.push_back("  - drop <tool>:        (drop)");
    lines.push_back("  - quit:               (exit game)");
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

    // Display and clear the message buffer
    if (!message_buffer.empty()) {
        std::cout << message_buffer << std::endl;
        message_buffer.clear();
        // Place cursor for input below the message
        console->setCursorPosition(0, max_height + 2);
    }
}

void Game::printWelcomeMessage() {
    std::cout << "Welcome to Quanta_Pie!" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    // displayGameScreen(); // Use the new display function
    // printHelp(); // Help is now part of the side panel
}

void Game::gameLoop() {
    while (!gameOver) {
        if (g_signal_received) { // Check for signal
            gameOver = true;
            continue;
        }
        displayGameScreen();
        std::cout << "> ";

#if defined(_WIN32) || defined(_WIN64)
        // This part is for Windows and supports arrow keys, but not multi-word commands.
        // The core logic is now in processInput, which expects full command lines.
        // For simplicity in this refactor, we will use getline for all platforms.
        // A more robust solution would handle char-by-char input and build a command string.
        std::string input_line;
        std::getline(std::cin, input_line);
        processInput(input_line);
#else
        std::string input_line;
        std::getline(std::cin, input_line);
        processInput(input_line);
#endif
    }
    std::cout << "Thank you for playing Quanta_Pie!" << std::endl;
}

// Helper function to convert a string to lowercase
std::string toLower(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
    return lower_str;
}

void Game::processInput(const std::string& input) {
    if (!player) return;

    std::string lowerInput = toLower(input);
    Room* currentRoom = player->getCurrentRoom();

    if (lowerInput == "quit") {
        gameOver = true;
        return;
    }

    // If a challenge is active, handle numbered choices
    if (current_challenge) {
        int choice_num = -1;
        try {
            choice_num = std::stoi(lowerInput);
            if (choice_num > 0 && static_cast<size_t>(choice_num) <= current_challenge->getChoices().size()) {
                current_challenge->getChoices()[choice_num - 1].action();
            } else {
                message_buffer = "Invalid choice.";
            }
        } catch (const std::invalid_argument&) {
            message_buffer = "Please enter a number to make a choice.";
        } catch (const std::out_of_range&) {
            message_buffer = "That number is out of range.";
        }
        return;
    }

    // Command Parsing
    if (lowerInput == "help") {
        message_buffer = "Help is displayed on the right panel.";
    } else if (lowerInput == "look") {
        // The screen redraws automatically, so this command just serves to refresh.
        // We can add a specific message if desired.
        message_buffer = "You look around.";
    } else if (lowerInput.rfind("look at ", 0) == 0) {
        std::string targetName = input.substr(8);
        bool found = false;
        for (const auto& object : currentRoom->getObjects()) {
            if (toLower(object->getName()) == toLower(targetName)) {
                message_buffer = object->getDescription();
                found = true;
                break;
            }
        }
        if (!found) {
            message_buffer = "You don't see any '" + targetName + "' here.";
        }
    } else if (lowerInput.rfind("take ", 0) == 0) {
        std::string targetName = input.substr(5);
        Tool* toolToTake = nullptr;
        for (auto& tool : currentRoom->getTools()) {
            if (toLower(tool->getName()) == toLower(targetName)) {
                toolToTake = tool;
                break;
            }
        }
        if (toolToTake) {
            player->takeTool(toolToTake);
            // player->takeTool already removes it from the room, so we don't need to do it here.
            message_buffer = "You take the " + targetName + ".";
        } else {
            message_buffer = "You can't find any '" + targetName + "' to take.";
        }
    } else if (lowerInput.rfind("drop ", 0) == 0) {
        std::string targetName = input.substr(5);
        Tool* toolToDrop = nullptr;
        for (auto& tool : player->getTools()) {
            if (toLower(tool->getName()) == toLower(targetName)) {
                toolToDrop = tool;
                break;
            }
        }
        if (toolToDrop) {
            player->dropTool(toolToDrop);
            message_buffer = "You drop the " + targetName + ".";
        } else {
            message_buffer = "You don't have a '" + targetName + "' to drop.";
        }
    } else {
        // Assume movement
        Room* nextRoom = currentRoom->getExit(lowerInput);
        if (nextRoom != nullptr) {
            player->setCurrentRoom(nextRoom);
            player->incrementScore();
            if (nextRoom->getChallenge() != nullptr) {
                current_challenge = std::make_unique<Challenge>(*nextRoom->getChallenge());
            }
        } else {
            message_buffer = "You can't do that. (Type 'help' for commands)";
        }
    }
}