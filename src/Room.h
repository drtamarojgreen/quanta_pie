#ifndef ROOM_H
#define ROOM_H

#include <string>
#include <map>

// Forward declaration to resolve circular dependency with Player
class Player;

class Room {
public:
    Room(const std::string& description);

    void addExit(const std::string& direction, Room* room);
    Room* getExit(const std::string& direction);
    std::string getDescription() const;
    void printExits() const;

private:
    std::string description;
    std::map<std::string, Room*> exits;
};

#endif // ROOM_H
