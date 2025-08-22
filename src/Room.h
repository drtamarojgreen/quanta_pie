#ifndef ROOM_H
#define ROOM_H

#include <string>
#include <map>
#include <vector>
#include <memory> // For std::unique_ptr

#include "objects/RoomObject.h"
#include "objects/Challenge.h"

// Forward declarations
class Player;
class Challenge;
class Tool;

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
     */
    Room(const std::string& description);
    ~Room(); // Destructor

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
     * @brief Prints the available exits from this room to the console.
     */
    void printExits() const;

    /**
     * @brief Adds an object to the room.
     * @param object A pointer to the RoomObject to add.
     */
    void addObject(RoomObject* object);

    /**
     * @brief Removes an object from the room.
     * @param object A pointer to the RoomObject to remove.
     */
    void removeObject(RoomObject* object);

    /**
     * @brief Gets all the objects in the room.
     * @return A constant reference to the vector of objects in the room.
     */
    const std::vector<RoomObject*>& getObjects() const;

    /**
     * @brief Adds a tool to the room.
     * @param tool A pointer to the Tool to add.
     */
    void addTool(Tool* tool);

    /**
     * @brief Removes a tool from the room.
     * @param tool A pointer to the Tool to remove.
     */
    void removeTool(Tool* tool);

    /**
     * @brief Gets all the tools in the room.
     * @return A constant reference to the vector of tools in the room.
     */
    const std::vector<Tool*>& getTools() const;

    const std::map<std::string, Room*>& getAllExits() const; // New function to get all exits
    void setChallenge(std::unique_ptr<Challenge> challenge); // Set a challenge for this room
    Challenge* getChallenge() const; // Get the challenge for this room

private:
    std::string description;
    std::map<std::string, Room*> exits;
    std::vector<RoomObject*> objects;
    std::vector<Tool*> tools;
    std::unique_ptr<Challenge> room_challenge; // Optional challenge for the room
};

#endif // ROOM_H