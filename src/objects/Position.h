#ifndef POSITION_H
#define POSITION_H

#include <cstdlib>
#include <string>

/**
 * @enum Direction
 * @brief The eight compass directions a character can step or look in.
 */
enum class Direction {
    North,
    NorthEast,
    East,
    SouthEast,
    South,
    SouthWest,
    West,
    NorthWest
};

/**
 * @struct Position
 * @brief A tile coordinate inside a room. x grows east, y grows south.
 *
 * A default-constructed Position is invalid, meaning "not placed on the grid".
 */
struct Position {
    int x = -1;
    int y = -1;

    constexpr Position() = default;
    constexpr Position(int column, int row) : x(column), y(row) {}

    bool isValid() const { return x >= 0 && y >= 0; }

    Position step(Direction direction) const {
        switch (direction) {
            case Direction::North:     return {x, y - 1};
            case Direction::NorthEast: return {x + 1, y - 1};
            case Direction::East:      return {x + 1, y};
            case Direction::SouthEast: return {x + 1, y + 1};
            case Direction::South:     return {x, y + 1};
            case Direction::SouthWest: return {x - 1, y + 1};
            case Direction::West:      return {x - 1, y};
            case Direction::NorthWest: return {x - 1, y - 1};
        }
        return *this;
    }

    // Chebyshev distance: the number of steps needed when diagonal moves are allowed.
    int distanceTo(const Position& other) const {
        int dx = std::abs(x - other.x);
        int dy = std::abs(y - other.y);
        return dx > dy ? dx : dy;
    }

    bool operator==(const Position& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Position& other) const { return !(*this == other); }
};

inline std::string directionName(Direction direction) {
    switch (direction) {
        case Direction::North:     return "north";
        case Direction::NorthEast: return "northeast";
        case Direction::East:      return "east";
        case Direction::SouthEast: return "southeast";
        case Direction::South:     return "south";
        case Direction::SouthWest: return "southwest";
        case Direction::West:      return "west";
        case Direction::NorthWest: return "northwest";
    }
    return "";
}

#endif // POSITION_H
