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
=======
class Player {
public:
    // Player's position vector
    double x;
    double y;

    // Player's direction vector
    double dirX;
    double dirY;

    // The camera plane, perpendicular to the direction
    double planeX;
    double planeY;

    /**
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
    Player(double startX, double startY);
};

#endif // PLAYER_H
