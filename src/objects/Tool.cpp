#include "Tool.h"

Tool::Tool(int id, const std::string& name, const std::string& description, int initial_room_id)
    : id(id), name(name), description(description), initial_room_id(initial_room_id) {
}

int Tool::getId() const {
    return id;
}

std::string Tool::getName() const {
    return name;
}

std::string Tool::getDescription() const {
    return description;
}

int Tool::getInitialRoomId() const {
    return initial_room_id;
}
