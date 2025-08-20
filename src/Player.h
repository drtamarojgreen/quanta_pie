#ifndef PLAYER_H
#define PLAYER_H

#include <string>

// Forward declaration of Room to avoid circular dependencies
class Room;

/**
 * @class Player
 * @brief Represents the player in the game.
 *
 * This class stores information about the player, such as their current
 * location. It provides methods to interact with and move the player
 * within the game world.
 */
class Player {
public:
    /**
     * @brief Constructs a new Player object.
     * @param startingRoom A pointer to the Room where the player will start.
     */
    Player(Room* startingRoom);

    /**
     * @brief Moves the player to a new room.
     * @param room A pointer to the new current Room.
     */
    void setCurrentRoom(Room* room);

    /**
     * @brief Gets the player's current room.
     * @return A pointer to the Room the player is currently in.
     */
    Room* getCurrentRoom() const;

    /**
     * @brief Gets a string representation of the player for display.
     * @return A string representing the player, e.g., "@".
     */
    std::string getRepresentation() const;

private:
    Room* currentRoom;
};

#endif // PLAYER_H
