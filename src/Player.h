#ifndef PLAYER_H
#define PLAYER_H

// Forward declaration of Room to avoid circular dependencies
class Room;

class Player {
public:
    Player(Room* startingRoom);

    void setCurrentRoom(Room* room);
    Room* getCurrentRoom() const;

private:
    Room* currentRoom;
};

#endif // PLAYER_H
