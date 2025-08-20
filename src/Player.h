#ifndef PLAYER_H
#define PLAYER_H

#include <vector>
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
 * location and other details from the database.
 */
class Player {
public:
    /**
     * @brief Constructs a new Player object.
     * @param id The player's unique ID.
     * @param name The player's name.
     * @param joinDate The date the player joined.
     * @param startingRoom A pointer to the Room where the player will start.
     */
    Player(int id, const std::string& name, const std::string& joinDate, Room* startingRoom);
lass Player {
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

<   const std::vector<std::string>& getInventory() const;
    void addItem(const std::string& item);
    void removeItem(const std::string& item);
=   /**
     * @brief Gets a string representation of the player for display.
     * @return A string representing the player, e.g., "@".
     * @brief Gets the player's ID.
     * @return The player's ID.
     */
    int getID() const;

    /**
     * @brief Gets the player's name.
     * @return The player's name.
     */
    std::string getName() const;

    /**
     * @brief Gets the player's join date.
     * @return The player's join date.
     */
    std::string getJoinDate() const;

    /**
     * @brief Gets a string representation of the player for display.
     * @return A string representing the player, e.g., "@".
     */
    std::string getRepresentation() const;

private:
    int id;
    std::string name;
    std::string joinDate;
    Room* currentRoom;
     * @brief Constructs a new Player object.
     * @param startX The player's starting x-coordinate.
     * @param startY The player's starting y-coordinate.
     */
    std::string getRepresentation() const;
>
private:
    Room* currentRoom;
    std::vector<std::string> inventory;
};

#endif // PLAYER_H
