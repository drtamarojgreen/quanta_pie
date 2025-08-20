#include "Room.h"
#include "Character.h"
#include <iostream>

Room::Room(const std::string& description) : description(description) {}

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

void Room::addCharacter(Character* character) {
    characters.push_back(character);
}

const std::vector<Character*>& Room::getCharacters() const {
    return characters;
}

void Room::printExits() const {
    std::cout << "Available exits:";
    for (auto const& [direction, room] : exits) {
        std::cout << " " << direction;
    }
    std::cout << std::endl;
}
