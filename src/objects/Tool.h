#ifndef TOOL_H
#define TOOL_H

#include "RoomObject.h"
#include <string>

class Tool : public RoomObject {
public:
    Tool(int id, const std::string& name, const std::string& description, int initial_room_id, int weight = 1);

    int getInitialRoomId() const;
    int getWeight() const;

private:
    int weight;
};

#endif // TOOL_H
