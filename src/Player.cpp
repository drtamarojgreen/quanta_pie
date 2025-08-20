#include "Player.h"

Player::Player(int id, const std::string& name, const std::string& joinDate, Room* startingRoom)
    : id(id), name(name), joinDate(joinDate), currentRoom(startingRoom) {}
/**
 * @brief Constructs a new Player object.
 *
 * Initializes the player's state, including position, direction vector,
 * and camera plane. The direction is initially set to look along the
 * negative x-axis, and the camera plane is set for a standard field of view.
 *
 * @param startX The player's starting x-coordinate.
 * @param startY The player's starting y-coordinate.
 */
Player::Player(double startX, double startY) {
    // Player's starting position
    x = startX;
    y = startY;

    // Initial direction vector (looking down the negative X axis)
    dirX = -1.0;
    dirY = 0.0;

int Player::getID() const {
    return id;
}

std::string Player::getName() const {
    return name;
}

std::string Player::getJoinDate() const {
    return joinDate;
}

std::string Player::getRepresentation() const {
    return "  __\n (..)\n-(--)-\n /  \\";
    // Camera plane (perpendicular to the direction vector)
    // This creates a 66-degree field of view.
    planeX = 0.0;
    planeY = 0.66;
}
