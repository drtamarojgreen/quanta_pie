#include "Player.h"
#include "Room.h"

Player::Player(Room* startingRoom) : currentRoom(startingRoom) {}

void Player::setCurrentRoom(Room* room) {
    currentRoom = room;
}

Room* Player::getCurrentRoom() const {
    return currentRoom;
}

std::string Player::getRepresentation() const {
    return "  __\n (..)\n-(--)-\n /  \\";
}
