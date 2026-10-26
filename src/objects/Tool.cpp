#include "Tool.h"

Tool::Tool(int id, const std::string& name, const std::string& description, int initial_room_id, int weight)
    : RoomObject(id, name, description, initial_room_id), weight(weight) {
}

int Tool::getInitialRoomId() const {
    return getRoomId();
}

int Tool::getWeight() const {
    return weight;
}
