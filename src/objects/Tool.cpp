#include "Tool.h"

Tool::Tool(int id, const std::string& name, const std::string& description, int initial_room_id)
    : RoomObject(id, name, description, initial_room_id) {
}
