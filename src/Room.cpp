#include "Room.h"
#include "Character.h"
#include "Challenge.h" // Include Challenge.h
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

const std::map<std::string, Room*>& Room::getAllExits() const {
    return exits;
}

void Room::setChallenge(std::unique_ptr<Challenge> challenge) {
    room_challenge = std::move(challenge);
}

Challenge* Room::getChallenge() const {
    return room_challenge.get();
}

void Room::printExits() const {
    std::cout << "Available exits:";
    for (auto const& [direction, room] : exits) {
        std::cout << " " << direction;
    }
    std::cout << std::endl;
}
