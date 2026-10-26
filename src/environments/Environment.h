#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../Room.h"
#include "RoomLayout.h"
#include "WorldData.h"
#include "../objects/Equipment.h"
#include "../objects/Obstacle.h"

/**
 * @class Environment
 * @brief The rooms of the gauntlet, built from WorldData loaded from sql/.
 *
 * Each room holds a row of obstacles that are worked through in order; the last one
 * stands on the layout's north door tile and the door opens once all are cleared. Only
 * the active obstacle's equipment is on the room's fixed spawn points at any time:
 * exactly the Emotional Intelligence and Resilience it needs. Clearing it consumes that
 * equipment and spawns the next obstacle's. Rooms connect through the exits in the data:
 * "north" leaves by the door, "south" by the entry.
 *
 * The Environment owns its rooms, equipment and obstacles.
 */
class Environment {
public:
    static const std::string kForward;
    static const std::string kBack;

    /**
     * @throws WorldDataError if the data fails validation.
     */
    explicit Environment(const WorldData& data);

    /**
     * @brief Loads the world tables from a directory (normally "sql") and builds the rooms.
     * @throws WorldDataError if the data is missing or invalid.
     */
    explicit Environment(const std::string& data_directory);

    ~Environment();

    Environment(const Environment&) = delete;
    Environment& operator=(const Environment&) = delete;

    int getRoomCount() const;
    Room* getStartingRoom() const;
    Room* getFinalRoom() const;
    Room* getRoom(int room_number) const; // 1-based; nullptr if out of range
    int getRoomNumber(const Room* room) const; // 0 if the room is not part of this environment
    std::string getRoomName(int room_number) const; // Empty if out of range
    std::string getExitDescription(int room_number, const std::string& direction) const; // Empty if no exit
    const RoomLayout& getLayout(int room_number) const; // 1-based; throws std::out_of_range

    int getObstacleCount() const; // Across every room
    const std::vector<Obstacle*>& getObstacles(int room_number) const; // In clearing order; empty if out of range
    Obstacle* getActiveObstacle(int room_number) const; // The first not yet cleared; nullptr once the room is clear
    bool isRoomCleared(int room_number) const;
    std::string getObstacleName(int type) const; // Empty if the data defines no such type
    char getObstacleSymbol(int type) const; // 0 if the data defines no such type

    const std::vector<Equipment*>& getEquipmentFor(const Obstacle* obstacle) const; // Its spawn wave
    /**
     * @brief Puts the active obstacle's equipment on the spawn points, unless it is already spawned.
     * @return How many pieces appeared.
     */
    int spawnEquipmentForActiveObstacle(int room_number);

    const std::vector<std::unique_ptr<Obstacle>>& getAllObstacles() const; // Room by room, in clearing order
    const std::vector<std::unique_ptr<Equipment>>& getEquipment() const; // Every piece, in creation order
    bool isComplete() const; // True once every obstacle has been cleared

private:
    std::vector<std::unique_ptr<Room>> rooms;
    std::vector<std::unique_ptr<Obstacle>> obstacles;
    std::vector<std::unique_ptr<Equipment>> equipment;
    std::vector<std::vector<Obstacle*>> room_obstacles; // One list per room, same order as rooms
    std::unordered_map<const Obstacle*, std::vector<Equipment*>> waves;
    std::vector<RoomLayout> layouts; // One per room, same order
    std::vector<std::string> room_names; // One per room, same order
    std::unordered_map<const Room*, int> room_numbers;
    std::map<std::pair<int, std::string>, std::string> exit_descriptions;
    std::map<int, ObstacleRecord> obstacle_kinds;

    void build(const WorldData& data);
};

#endif // ENVIRONMENT_H
