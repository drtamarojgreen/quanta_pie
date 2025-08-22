#include "Player.h"
#include "../Room.h"

Player::Player(int id, const std::string& name, const std::string& joinDate, Room* startingRoom)
    : id(id), name(name), joinDate(joinDate), currentRoom(startingRoom), score(0) {}

Player::Player(Room* startingRoom)
    : id(0), name("Player"), joinDate("N/A"), currentRoom(startingRoom), score(0) {}

void Player::move(const std::string& direction) {
    if (currentRoom) {
        Room* nextRoom = currentRoom->getExit(direction);
        if (nextRoom) {
            setCurrentRoom(nextRoom);
        }
    }
}

void Player::setCurrentRoom(Room* room) {
    currentRoom = room;
}

Room* Player::getCurrentRoom() const {
    return currentRoom;
}

int Player::getID() const {
    return id;
}

std::string Player::getName() const {
    return name;
}

std::string Player::getJoinDate() const {
    return joinDate;
}

std::string Player::getRepresentation() const {
    return "@";
}

int Player::getScore() const {
    return score;
}

void Player::incrementScore(int amount) {
    score += amount;
}

void Player::takeTool(Tool* tool) {
    if (tool) {
        tools.push_back(tool);
        if (currentRoom) {
            currentRoom->removeObject(tool);
        }
    }
}

const std::vector<Tool*>& Player::getTools() const {
    return tools;
}

void Player::dropTool(Tool* tool) {
    if (tool && currentRoom) {
        // Remove the tool from the player's inventory
        tools.erase(std::remove(tools.begin(), tools.end(), tool), tools.end());
        // Add the tool to the current room
        currentRoom->addObject(tool);
    }
}