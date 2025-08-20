#include "Player.h"

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

    // Camera plane (perpendicular to the direction vector)
    // This creates a 66-degree field of view.
    planeX = 0.0;
    planeY = 0.66;
}
