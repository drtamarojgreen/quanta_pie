#ifndef PLAYER_H
#define PLAYER_H

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
     * @brief Constructs a new Player object.
     * @param startX The player's starting x-coordinate.
     * @param startY The player's starting y-coordinate.
     */
    Player(double startX, double startY);
};

#endif // PLAYER_H
