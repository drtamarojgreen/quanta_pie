#ifndef PLAYER_H
#define PLAYER_H

#include <string>

class Room;

class Player {
public:
    Player(int id, const std::string& name, const std::string& joinDate, Room* startingRoom);
    Player(Room* startingRoom);

    void setCurrentRoom(Room* room);
    Room* getCurrentRoom() const;
    int getID() const;
    std::string getName() const;
    std::string getJoinDate() const;
    std::string getRepresentation() const;
    int getScore() const;
    void incrementScore(int amount = 1);

private:
    int id;
    std::string name;
    std::string joinDate;
    Room* currentRoom;
    int score; // Added for tracking player score
};

#endif // PLAYER_H
