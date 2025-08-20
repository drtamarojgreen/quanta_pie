#ifndef PLAYER_H
#define PLAYER_H

#include <string>

// Forward declaration of Room to avoid circular dependencies
class Room;

class Player {
public:
    Player(Room* startingRoom);

    void setCurrentRoom(Room* room);
    Room* getCurrentRoom() const;
    std::string getRepresentation() const;

private:
    Room* currentRoom;
};

#endif // PLAYER_H
