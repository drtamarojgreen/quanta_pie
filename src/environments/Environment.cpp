#include "Environment.h"
#include <stdexcept>

namespace {

const std::vector<Obstacle*> kNoObstacles;
const std::vector<Equipment*> kNoEquipment;

} // namespace

const std::string Environment::kForward = "north";
const std::string Environment::kBack = "south";

Environment::Environment(const WorldData& data) {
    data.validate();
    build(data);
}

Environment::Environment(const std::string& data_directory)
    : Environment(WorldData::loadFromDirectory(data_directory)) {
}

Environment::~Environment() = default;

void Environment::build(const WorldData& data) {
    // Allocate everything up front; nothing is created or destroyed during play.
    size_t room_count = data.rooms.size();
    size_t obstacle_count = 0;
    size_t equipment_count = 0;
    for (const RoomRecord& record : data.rooms) {
        obstacle_count += record.obstacle_types.size();
        for (int type : record.obstacle_types) {
            equipment_count += Obstacle::requiredEmotionalIntelligenceFor(type) + Obstacle::requiredResilienceFor(type);
        }
    }
    rooms.reserve(room_count);
    room_obstacles.reserve(room_count);
    layouts.reserve(room_count);
    room_names.reserve(room_count);
    obstacles.reserve(obstacle_count);
    equipment.reserve(equipment_count);
    obstacle_kinds = data.obstacles;

    int next_object_id = 1;
    for (const RoomRecord& record : data.rooms) {
        int number = record.id;
        const RoomLayout& layout = data.layouts.at(record.layout_id).layout;
        auto room = std::make_unique<Room>(record.description);
        std::vector<Obstacle*> in_room;

        for (size_t i = 0; i < record.obstacle_types.size(); ++i) {
            const ObstacleRecord& kind = data.obstacles.at(record.obstacle_types[i]);
            obstacles.push_back(std::make_unique<Obstacle>(next_object_id++, kind.type, number, kind.name,
                                                           kind.description, kind.symbol, kForward));
            Obstacle* obstacle = obstacles.back().get();
            bool last = i + 1 == record.obstacle_types.size();
            obstacle->setPosition(last ? layout.door : layout.obstacle_points[i]);
            room->addObject(obstacle);
            in_room.push_back(obstacle);

            // The obstacle's wave waits off the map until it becomes the active obstacle.
            std::vector<Equipment*>& wave = waves[obstacle];
            auto prepare = [&](EquipmentKind equipment_kind, int count, const std::vector<Position>& spawns) {
                for (int slot = 1; slot <= count; ++slot) {
                    const EquipmentRecord& item = data.equipmentFor(equipment_kind, slot);
                    equipment.push_back(std::make_unique<Equipment>(next_object_id++, item.name, item.description,
                                                                    number, equipment_kind));
                    equipment.back()->setPosition(spawns[slot - 1]);
                    wave.push_back(equipment.back().get());
                }
            };
            prepare(EquipmentKind::EmotionalIntelligence, obstacle->getRequiredEmotionalIntelligence(), layout.ei_spawns);
            prepare(EquipmentKind::Resilience, obstacle->getRequiredResilience(), layout.r_spawns);
        }

        room_numbers[room.get()] = number;
        room_names.push_back(record.name);
        room_obstacles.push_back(std::move(in_room));
        rooms.push_back(std::move(room));
        layouts.push_back(layout);
        spawnEquipmentForActiveObstacle(number);
    }

    for (const ExitRecord& exit : data.exits) {
        rooms[exit.from_room_id - 1]->addExit(exit.direction, rooms[exit.to_room_id - 1].get());
        exit_descriptions[{exit.from_room_id, exit.direction}] = exit.description;
    }
}

int Environment::getRoomCount() const {
    return static_cast<int>(rooms.size());
}

Room* Environment::getStartingRoom() const {
    return rooms.front().get();
}

Room* Environment::getFinalRoom() const {
    return rooms.back().get();
}

Room* Environment::getRoom(int room_number) const {
    if (room_number < 1 || room_number > getRoomCount()) {
        return nullptr;
    }
    return rooms[room_number - 1].get();
}

int Environment::getRoomNumber(const Room* room) const {
    auto found = room_numbers.find(room);
    return found == room_numbers.end() ? 0 : found->second;
}

std::string Environment::getRoomName(int room_number) const {
    if (room_number < 1 || room_number > getRoomCount()) {
        return "";
    }
    return room_names[room_number - 1];
}

std::string Environment::getExitDescription(int room_number, const std::string& direction) const {
    auto found = exit_descriptions.find({room_number, direction});
    return found == exit_descriptions.end() ? "" : found->second;
}

const RoomLayout& Environment::getLayout(int room_number) const {
    if (room_number < 1 || room_number > getRoomCount()) {
        throw std::out_of_range("Room number outside environment");
    }
    return layouts[room_number - 1];
}

int Environment::getObstacleCount() const {
    return static_cast<int>(obstacles.size());
}

const std::vector<Obstacle*>& Environment::getObstacles(int room_number) const {
    if (room_number < 1 || room_number > getRoomCount()) {
        return kNoObstacles;
    }
    return room_obstacles[room_number - 1];
}

Obstacle* Environment::getActiveObstacle(int room_number) const {
    for (Obstacle* obstacle : getObstacles(room_number)) {
        if (!obstacle->isCleared()) {
            return obstacle;
        }
    }
    return nullptr;
}

bool Environment::isRoomCleared(int room_number) const {
    return getRoom(room_number) != nullptr && getActiveObstacle(room_number) == nullptr;
}

std::string Environment::getObstacleName(int type) const {
    auto found = obstacle_kinds.find(type);
    return found == obstacle_kinds.end() ? "" : found->second.name;
}

char Environment::getObstacleSymbol(int type) const {
    auto found = obstacle_kinds.find(type);
    return found == obstacle_kinds.end() ? 0 : found->second.symbol;
}

const std::vector<Equipment*>& Environment::getEquipmentFor(const Obstacle* obstacle) const {
    auto found = waves.find(obstacle);
    return found == waves.end() ? kNoEquipment : found->second;
}

int Environment::spawnEquipmentForActiveObstacle(int room_number) {
    Obstacle* active = getActiveObstacle(room_number);
    int spawned = 0;
    for (Equipment* piece : getEquipmentFor(active)) {
        if (!piece->isSpawned()) {
            piece->spawn();
            rooms[room_number - 1]->addObject(piece);
            spawned++;
        }
    }
    return spawned;
}

const std::vector<std::unique_ptr<Obstacle>>& Environment::getAllObstacles() const {
    return obstacles;
}

const std::vector<std::unique_ptr<Equipment>>& Environment::getEquipment() const {
    return equipment;
}

bool Environment::isComplete() const {
    for (const auto& obstacle : obstacles) {
        if (!obstacle->isCleared()) {
            return false;
        }
    }
    return true;
}
