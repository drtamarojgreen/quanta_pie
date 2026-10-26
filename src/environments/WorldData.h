#ifndef WORLD_DATA_H
#define WORLD_DATA_H

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "RoomLayout.h"
#include "../objects/Equipment.h"

/**
 * @class WorldDataError
 * @brief Thrown when the world data is missing or inconsistent. The message names the file and row.
 */
class WorldDataError : public std::runtime_error {
public:
    explicit WorldDataError(const std::string& message) : std::runtime_error(message) {}
};

struct RoomRecord {
    int id = 0;
    std::string name;
    std::string description;
    int layout_id = 0;
    std::vector<int> obstacle_types; // In clearing order, from gauntlet_room_obstacles.csv
};

struct LayoutRecord {
    int id = 0;
    std::string name;
    RoomLayout layout; // Points are ordered by slot: ei_spawns[0] and obstacle_points[0] are slot 1
};

struct ExitRecord {
    int id = 0;
    int from_room_id = 0;
    int to_room_id = 0;
    std::string direction;
    std::string description;
};

struct ObstacleRecord {
    int type = 0;
    char symbol = 0; // Drawn on the map; unique to the type
    std::string name;
    std::string description;
};

struct EquipmentRecord {
    int id = 0;
    EquipmentKind kind = EquipmentKind::EmotionalIntelligence;
    int slot = 0; // The nth piece of this kind a room needs
    std::string name;
    std::string description;
};

/**
 * @struct WorldData
 * @brief The rooms and locations of the gauntlet, as loaded from the CSV tables in sql/.
 *
 * Tables (see sql/game_data.sql for the schema):
 *   gauntlet_rooms.csv            room_id,name,description,layout_id
 *   gauntlet_room_obstacles.csv   room_obstacle_id,room_id,sequence,obstacle_type
 *   gauntlet_layouts.csv          layout_id,name,width,height,door_x,door_y,entry_x,entry_y
 *   gauntlet_spawn_points.csv     spawn_point_id,layout_id,kind,slot,x,y
 *   gauntlet_obstacle_points.csv  obstacle_point_id,layout_id,slot,x,y
 *   gauntlet_hazards.csv          hazard_id,layout_id,x,y,damage,name,description
 *   gauntlet_exits.csv            exit_id,from_room_id,to_room_id,direction,description
 *   gauntlet_obstacles.csv        obstacle_type,symbol,name,description
 *   gauntlet_equipment.csv        equipment_id,kind,slot,name,description
 */
struct WorldData {
    static const char* const kRoomsFile;
    static const char* const kRoomObstaclesFile;
    static const char* const kLayoutsFile;
    static const char* const kSpawnPointsFile;
    static const char* const kObstaclePointsFile;
    static const char* const kHazardsFile;
    static const char* const kExitsFile;
    static const char* const kObstaclesFile;
    static const char* const kEquipmentFile;

    std::vector<RoomRecord> rooms; // Sorted, so rooms[i].id == i + 1
    std::map<int, LayoutRecord> layouts;
    std::vector<ExitRecord> exits;
    std::map<int, ObstacleRecord> obstacles; // By obstacle type
    std::map<int, EquipmentRecord> ei_equipment; // By slot
    std::map<int, EquipmentRecord> r_equipment;  // By slot

    /**
     * @brief Reads and validates every table in a directory.
     * @throws WorldDataError if a file is missing, a row is malformed or the data is inconsistent.
     */
    static WorldData loadFromDirectory(const std::string& directory);

    /**
     * @brief Checks that every room can be built and played.
     * @throws WorldDataError describing the first problem found.
     */
    void validate() const;

    const EquipmentRecord& equipmentFor(EquipmentKind kind, int slot) const;

    // Characters the map already uses, so no obstacle may take them as its symbol
    static const char* const kReservedSymbols;
};

#endif // WORLD_DATA_H
