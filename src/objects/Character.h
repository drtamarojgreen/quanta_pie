#ifndef CHARACTER_H
#define CHARACTER_H

#include "RoomObject.h"
#include <string>

// Forward declaration of Room to avoid circular dependency
class Room;

class Character : public RoomObject {
public:
    Character(int id, const std::string& name, const std::string& description, int room_id, const std::string& dialogue);

    std::string getDialogue() const;

private:
    std::string dialogue;
};

#endif // CHARACTER_H
