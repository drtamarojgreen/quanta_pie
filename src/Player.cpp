#include "Player.h"
#include "Room.h"
#include <algorithm> // For std::find

Player::Player(Room* startingRoom) : currentRoom(startingRoom) {
    // Inventory is default-initialized to empty
}


void Player::setCurrentRoom(Room* room) {
    currentRoom = room;
}

Room* Player::getCurrentRoom() const {
    return currentRoom;
}

const std::vector<std::string>& Player::getInventory() const {
    return inventory;
}

void Player::addItem(const std::string& item) {
    inventory.push_back(item);
}

void Player::removeItem(const std::string& item) {
    auto it = std::find(inventory.begin(), inventory.end(), item);
    if (it != inventory.end()) {
        inventory.erase(it);
    }
}
