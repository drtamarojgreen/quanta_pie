#ifndef ROOM_H
#define ROOM_H

#include <string>
#include <map>

// Forward declaration to resolve circular dependency with Player
class Player;

/**
 * @class Room
 * @brief Represents a single location or room in the game world.
 *
 * This class stores details about a room, including its description and
 * the exits that connect it to other rooms.
 */
class Room {
public:
    /**
     * @brief Constructs a new Room object.
     * @param description A text description of the room.
     * @param x The x-coordinate of the room in 3D space.
     * @param y The y-coordinate of the room in 3D space.
     * @param z The z-coordinate of the room in 3D space.
     */
    Room(const std::string& description, int x = 0, int y = 0, int z = 0);

    /**
     * @brief Adds an exit to the room.
     * @param direction The direction of the exit (e.g., "north", "south").
     * @param room A pointer to the Room this exit leads to.
     */
    void addExit(const std::string& direction, Room* room);

    /**
     * @brief Gets the room connected by an exit in a specific direction.
     * @param direction The direction to check for an exit.
     * @return A pointer to the connected Room, or nullptr if no exit exists.
     */
    Room* getExit(const std::string& direction);

    /**
     * @brief Gets the description of the room.
     * @return The room's description string.
     */
    std::string getDescription() const;

    /**
     * @brief Gets the x-coordinate of the room.
     * @return The x-coordinate.
     */
    int getX() const;

    /**
     * @brief Gets the y-coordinate of the room.
     * @return The y-coordinate.
     */
    int getY() const;

    /**
     * @brief Gets the z-coordinate of the room.
     * @return The z-coordinate.
     */
    int getZ() const;

    /**
     * @brief Prints the available exits from this room to the console.
     */
    void printExits() const;

private:
    std::string description;
    std::map<std::string, Room*> exits;
    int x, y, z; // 3D coordinates of the room
};

#endif // ROOM_H
