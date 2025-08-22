#include "Room.h"
#include "objects/RoomObject.h"
#include "objects/Challenge.h" // Include Challenge.h
#include <iostream>
#include <algorithm> // For std::remove

Room::Room(const std::string& description) : description(description) {}

Room::~Room() = default;

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

void Room::addObject(RoomObject* object) {
    objects.push_back(object);
}

void Room::removeObject(RoomObject* object) {
    objects.erase(std::remove(objects.begin(), objects.end(), object), objects.end());
}

const std::vector<RoomObject*>& Room::getObjects() const {
    return objects;
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
