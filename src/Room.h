#ifndef ROOM_H
#define ROOM_H

#include <string>
#include <map>
#include <vector>

// Forward declarations
class Player;
class Character;

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
     * @brief Adds a character to the room.
     * @param character A pointer to the Character to add to the room.
     */
    void addCharacter(Character* character);

    /**
     * @brief Gets the characters in the room.
     * @return A constant reference to the vector of characters in the room.
     */
    const std::vector<Character*>& getCharacters() const;
    const std::map<std::string, Room*>& getAllExits() const; // New function to get all exits
    void setChallenge(std::unique_ptr<Challenge> challenge); // Set a challenge for this room
    Challenge* getChallenge() const; // Get the challenge for this room

private:
    std::string description;
    std::map<std::string, Room*> exits;
    std::vector<Character*> characters;
    std::unique_ptr<Challenge> room_challenge; // Optional challenge for the room
};

#endif // ROOM_H