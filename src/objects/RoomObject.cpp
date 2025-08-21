#include "RoomObject.h"

RoomObject::RoomObject(int id, const std::string& name, const std::string& description, int room_id)
    : id(id), name(name), description(description), room_id(room_id) {
}

int RoomObject::getId() const {
    return id;
}

std::string RoomObject::getName() const {
    return name;
}

std::string RoomObject::getDescription() const {
    return description;
}

int RoomObject::getRoomId() const {
    return room_id;
}
