#ifndef ROOM_OBJECT_H
#define ROOM_OBJECT_H

#include <string>

class RoomObject {
public:
    RoomObject(int id, const std::string& name, const std::string& description, int room_id);

    int getId() const;
    std::string getName() const;
    std::string getDescription() const;
    int getRoomId() const;

private:
    int id;
    std::string name;
    std::string description;
    int room_id;
};

#endif // ROOM_OBJECT_H
