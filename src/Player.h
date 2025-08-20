#ifndef PLAYER_H
#define PLAYER_H

#include <vector>
#include <string>

// Forward declaration of Room to avoid circular dependencies
class Room;

class Player {
public:
    Player(Room* startingRoom);

    void setCurrentRoom(Room* room);
    Room* getCurrentRoom() const;

    const std::vector<std::string>& getInventory() const;
    void addItem(const std::string& item);
    void removeItem(const std::string& item);

private:
    Room* currentRoom;
    std::vector<std::string> inventory;
};

#endif // PLAYER_H
