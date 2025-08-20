#include "Room.h"
#include <iostream>

Room::Room(const std::string& description, int x, int y, int z)
    : description(description), x(x), y(y), z(z) {}

void Room::addExit(const std::string& direction, Room* room) {
    exits[direction] = room;
}

Room* Room::getExit(const std::string& direction) {
    if (exits.count(direction)) {
        return exits[direction];
    }
    return nullptr;
}

std::string Room::getDescription() const {
    return description;
}

int Room::getX() const {
    return x;
}

int Room::getY() const {
    return y;
}

int Room::getZ() const {
    return z;
}

void Room::printExits() const {
    std::cout << "Available exits:";
    for (auto const& [direction, room] : exits) {
        std::cout << " " << direction;
    }
    std::cout << std::endl;
}
