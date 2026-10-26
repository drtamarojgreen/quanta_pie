#ifndef ROOM_LAYOUT_H
#define ROOM_LAYOUT_H

#include <string>
#include <vector>
#include "../objects/Position.h"

/**
 * @struct Hazard
 * @brief A fixed tile that costs extra health to step on.
 */
struct Hazard {
    Position position;
    int damage = 0;
    std::string name;
    std::string description;
};

/**
 * @struct RoomLayout
 * @brief The tile grid of a room and its fixed points, loaded from sql/gauntlet_layouts.csv,
 *        sql/gauntlet_spawn_points.csv, sql/gauntlet_obstacle_points.csv and sql/gauntlet_hazards.csv.
 *
 * A room's last obstacle stands on the north door tile; the ones before it stand on
 * obstacle_points[0], [1] and so on. Equipment always spawns on the same tiles: the
 * nth EI piece an obstacle needs goes on ei_spawns[n-1], and likewise for R.
 *
 * The shipped "Standard" layout:
 *   +------^------+
 *   |.97531X2468..|   X  last obstacle on the door tile; 1-9 the other obstacle points
 *   |.............|
 *   |..E.E.E.E.E..|   E  EI spawn points
 *   |...:.....:...|   :  hazards
 *   |..R.R.R.R.R..|   R  R spawn points
 *   |......:......|
 *   |......@......|   @  entry tile
 *   +------v------+
 */
struct RoomLayout {
    int width = 0;
    int height = 0;
    Position door;  // North exit; the room's obstacle stands here
    Position entry; // South exit; where the player arrives from the previous room
    std::vector<Position> ei_spawns;
    std::vector<Position> r_spawns;
    std::vector<Position> obstacle_points; // For every obstacle but the last, in clearing order
    std::vector<Hazard> hazards;

    const Hazard* hazardAt(const Position& position) const {
        for (const Hazard& hazard : hazards) {
            if (hazard.position == position) {
                return &hazard;
            }
        }
        return nullptr;
    }

    bool contains(const Position& position) const {
        return position.x >= 0 && position.y >= 0 && position.x < width && position.y < height;
    }
};

#endif // ROOM_LAYOUT_H
