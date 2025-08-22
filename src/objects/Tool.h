#ifndef TOOL_H
#define TOOL_H

#include "RoomObject.h"
#include <string>

class Tool : public RoomObject {
public:
    Tool(int id, const std::string& name, const std::string& description, int initial_room_id);
};

#endif // TOOL_H
