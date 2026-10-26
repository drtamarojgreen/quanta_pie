#ifndef ROOM_OBJECT_H
#define ROOM_OBJECT_H

#include <string>
#include "Position.h"

class RoomObject {
public:
    RoomObject(int id, const std::string& name, const std::string& description, int room_id);
    virtual ~RoomObject() = default;

    int getId() const;
    std::string getName() const;
    std::string getDescription() const;
    int getRoomId() const;

    // Tile inside the room; invalid until the object is placed on the grid
    Position getPosition() const;
    void setPosition(const Position& position);

private:
    int id;
    std::string name;
    std::string description;
    int room_id;
    Position position;
};

#endif // ROOM_OBJECT_H
