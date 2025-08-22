#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <vector>
#include "../objects/Tool.h"

class Room; // Forward declaration for Room

class Player {
public:
    Player(int id, const std::string& name, const std::string& joinDate, Room* startingRoom);
    Player(Room* startingRoom);

    void move(const std::string& direction); // For testing purposes
    void setCurrentRoom(Room* room);
    Room* getCurrentRoom() const;
    int getID() const;
    std::string getName() const;
    std::string getJoinDate() const;
    std::string getRepresentation() const;
    int getScore() const;
    void incrementScore(int amount = 1);

    // Inventory methods
    void takeTool(Tool* tool);
    void dropTool(Tool* tool);
    const std::vector<Tool*>& getTools() const;

private:
    int id;
    std::string name;
    std::string joinDate;
    Room* currentRoom;
    int score; // Added for tracking player score
    std::vector<Tool*> tools; // Player's inventory of tools
};

#endif // PLAYER_H
