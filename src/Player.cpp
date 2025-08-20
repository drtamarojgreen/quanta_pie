#include "Player.h"
#include "Room.h"

Player::Player(int id, const std::string& name, const std::string& joinDate, Room* startingRoom)
    : id(id), name(name), joinDate(joinDate), currentRoom(startingRoom) {}

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
    return "  __\n (..)\n-(--)-\n /  \\";
}
