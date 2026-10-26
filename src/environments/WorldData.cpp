#include "WorldData.h"
#include "../CSVParser.h"
#include "../objects/Obstacle.h"
#include <algorithm>
#include <cstring>
#include <set>

namespace {

struct Row {
    size_t line;
    std::vector<std::string> fields;
};

std::string trim(const std::string& text) {
    size_t begin = text.find_first_not_of(" \t");
    if (begin == std::string::npos) {
        return "";
    }
    size_t end = text.find_last_not_of(" \t");
    return text.substr(begin, end - begin + 1);
}

/**
 * Reads a table, checks its header has the expected columns, and returns the data rows
 * with their line numbers. Blank lines are skipped.
 */
std::vector<Row> readTable(const std::string& directory, const std::string& file,
                           const std::vector<std::string>& columns) {
    std::vector<std::vector<std::string>> lines = CSVParser::readCSV(directory + "/" + file);
    if (lines.empty()) {
        throw WorldDataError(file + ": missing or empty (looked in " + directory + ")");
    }
    std::vector<std::string> header;
    for (const std::string& field : lines[0]) {
        header.push_back(trim(field));
    }
    if (header != columns) {
        std::string expected;
        for (const std::string& column : columns) {
            expected += (expected.empty() ? "" : ",") + column;
        }
        throw WorldDataError(file + " line 1: header must be " + expected);
    }

    std::vector<Row> rows;
    for (size_t i = 1; i < lines.size(); ++i) {
        if (lines[i].size() == 1 && trim(lines[i][0]).empty()) {
            continue;
        }
        if (lines[i].size() != columns.size()) {
            throw WorldDataError(file + " line " + std::to_string(i + 1) + ": expected " +
                                 std::to_string(columns.size()) + " fields, found " + std::to_string(lines[i].size()));
        }
        Row row{i + 1, {}};
        for (const std::string& field : lines[i]) {
            row.fields.push_back(trim(field));
        }
        rows.push_back(row);
    }
    return rows;
}

std::string where(const std::string& file, const Row& row) {
    return file + " line " + std::to_string(row.line) + ": ";
}

int toInt(const std::string& file, const Row& row, size_t column, const char* name) {
    const std::string& field = row.fields[column];
    size_t used = 0;
    int value = 0;
    try {
        value = std::stoi(field, &used);
    } catch (const std::exception&) {
        used = 0;
    }
    if (field.empty() || used != field.size()) {
        throw WorldDataError(where(file, row) + name + " must be a whole number, found '" + field + "'");
    }
    return value;
}

EquipmentKind toKind(const std::string& file, const Row& row, size_t column) {
    const std::string& field = row.fields[column];
    if (field == "EI") {
        return EquipmentKind::EmotionalIntelligence;
    }
    if (field == "R") {
        return EquipmentKind::Resilience;
    }
    throw WorldDataError(where(file, row) + "kind must be EI or R, found '" + field + "'");
}

std::string label(EquipmentKind kind) {
    return kind == EquipmentKind::EmotionalIntelligence ? "EI" : "R";
}

std::string text(const Position& position) {
    return "(" + std::to_string(position.x) + "," + std::to_string(position.y) + ")";
}

// Places numbered slots into a list, rejecting gaps: slot n must be followed by slot n + 1.
void placeSlots(const std::map<int, Position>& slots, std::vector<Position>& points,
                const std::string& file, int layout_id, const std::string& what) {
    for (const auto& [slot, position] : slots) {
        if (slot != static_cast<int>(points.size()) + 1) {
            throw WorldDataError(file + ": layout " + std::to_string(layout_id) + " is missing " + what +
                                 " slot " + std::to_string(points.size() + 1));
        }
        points.push_back(position);
    }
}

} // namespace

const char* const WorldData::kRoomsFile = "gauntlet_rooms.csv";
const char* const WorldData::kRoomObstaclesFile = "gauntlet_room_obstacles.csv";
const char* const WorldData::kLayoutsFile = "gauntlet_layouts.csv";
const char* const WorldData::kSpawnPointsFile = "gauntlet_spawn_points.csv";
const char* const WorldData::kObstaclePointsFile = "gauntlet_obstacle_points.csv";
const char* const WorldData::kHazardsFile = "gauntlet_hazards.csv";
const char* const WorldData::kExitsFile = "gauntlet_exits.csv";
const char* const WorldData::kObstaclesFile = "gauntlet_obstacles.csv";
const char* const WorldData::kEquipmentFile = "gauntlet_equipment.csv";
const char* const WorldData::kReservedSymbols = ".@ER#+-|^v:";

WorldData WorldData::loadFromDirectory(const std::string& directory) {
    WorldData data;

    for (const Row& row : readTable(directory, kRoomsFile, {"room_id", "name", "description", "layout_id"})) {
        RoomRecord room;
        room.id = toInt(kRoomsFile, row, 0, "room_id");
        room.name = row.fields[1];
        room.description = row.fields[2];
        room.layout_id = toInt(kRoomsFile, row, 3, "layout_id");
        data.rooms.push_back(room);
    }
    std::sort(data.rooms.begin(), data.rooms.end(),
              [](const RoomRecord& a, const RoomRecord& b) { return a.id < b.id; });
    for (size_t i = 0; i < data.rooms.size(); ++i) {
        if (data.rooms[i].id != static_cast<int>(i) + 1) {
            throw WorldDataError(std::string(kRoomsFile) + ": room ids must run 1, 2, 3... without gaps or repeats; "
                                 "expected room " + std::to_string(i + 1) + ", found " + std::to_string(data.rooms[i].id));
        }
    }

    // A room's obstacles may be listed in any order; they are placed by sequence, then checked for gaps.
    std::map<int, std::map<int, int>> sequences; // room id -> sequence -> obstacle type
    for (const Row& row : readTable(directory, kRoomObstaclesFile, {"room_obstacle_id", "room_id", "sequence", "obstacle_type"})) {
        int room_id = toInt(kRoomObstaclesFile, row, 1, "room_id");
        int sequence = toInt(kRoomObstaclesFile, row, 2, "sequence");
        int type = toInt(kRoomObstaclesFile, row, 3, "obstacle_type");
        if (room_id < 1 || room_id > static_cast<int>(data.rooms.size())) {
            throw WorldDataError(where(kRoomObstaclesFile, row) + "room " + std::to_string(room_id) + " does not exist");
        }
        if (!Obstacle::isValidType(type)) {
            throw WorldDataError(where(kRoomObstaclesFile, row) + "obstacle_type must be 1-10");
        }
        if (sequence < 1 || !sequences[room_id].emplace(sequence, type).second) {
            throw WorldDataError(where(kRoomObstaclesFile, row) + "room " + std::to_string(room_id) +
                                 " already has an obstacle at sequence " + std::to_string(sequence) +
                                 " (sequences start at 1)");
        }
    }
    for (RoomRecord& room : data.rooms) {
        for (const auto& [sequence, type] : sequences[room.id]) {
            if (sequence != static_cast<int>(room.obstacle_types.size()) + 1) {
                throw WorldDataError(std::string(kRoomObstaclesFile) + ": room " + std::to_string(room.id) +
                                     " is missing sequence " + std::to_string(room.obstacle_types.size() + 1));
            }
            room.obstacle_types.push_back(type);
        }
    }

    for (const Row& row : readTable(directory, kLayoutsFile,
                                    {"layout_id", "name", "width", "height", "door_x", "door_y", "entry_x", "entry_y"})) {
        LayoutRecord record;
        record.id = toInt(kLayoutsFile, row, 0, "layout_id");
        record.name = row.fields[1];
        record.layout.width = toInt(kLayoutsFile, row, 2, "width");
        record.layout.height = toInt(kLayoutsFile, row, 3, "height");
        record.layout.door = {toInt(kLayoutsFile, row, 4, "door_x"), toInt(kLayoutsFile, row, 5, "door_y")};
        record.layout.entry = {toInt(kLayoutsFile, row, 6, "entry_x"), toInt(kLayoutsFile, row, 7, "entry_y")};
        if (!data.layouts.emplace(record.id, record).second) {
            throw WorldDataError(where(kLayoutsFile, row) + "layout " + std::to_string(record.id) + " is defined twice");
        }
    }

    // Reads a slot and a tile for an existing layout, checking the tile is on its grid.
    auto readPoint = [&data](const char* file, const Row& row, size_t layout_column, size_t slot_column,
                             int& layout_id, int& slot, Position& position) {
        layout_id = toInt(file, row, layout_column, "layout_id");
        slot = toInt(file, row, slot_column, "slot");
        position = {toInt(file, row, slot_column + 1, "x"), toInt(file, row, slot_column + 2, "y")};
        auto layout = data.layouts.find(layout_id);
        if (layout == data.layouts.end()) {
            throw WorldDataError(where(file, row) + "layout " + std::to_string(layout_id) + " does not exist");
        }
        if (slot < 1) {
            throw WorldDataError(where(file, row) + "slot must be 1 or more");
        }
        if (!layout->second.layout.contains(position)) {
            throw WorldDataError(where(file, row) + "point " + text(position) + " is outside layout " +
                                 std::to_string(layout_id));
        }
    };

    std::map<int, std::map<int, Position>> ei_slots;
    std::map<int, std::map<int, Position>> r_slots;
    for (const Row& row : readTable(directory, kSpawnPointsFile, {"spawn_point_id", "layout_id", "kind", "slot", "x", "y"})) {
        int layout_id = 0;
        int slot = 0;
        Position position;
        EquipmentKind kind = toKind(kSpawnPointsFile, row, 2);
        readPoint(kSpawnPointsFile, row, 1, 3, layout_id, slot, position);
        auto& slots = kind == EquipmentKind::EmotionalIntelligence ? ei_slots[layout_id] : r_slots[layout_id];
        if (!slots.emplace(slot, position).second) {
            throw WorldDataError(where(kSpawnPointsFile, row) + label(kind) + " slot " + std::to_string(slot) +
                                 " is defined twice for layout " + std::to_string(layout_id));
        }
    }

    std::map<int, std::map<int, Position>> obstacle_slots;
    for (const Row& row : readTable(directory, kObstaclePointsFile, {"obstacle_point_id", "layout_id", "slot", "x", "y"})) {
        int layout_id = 0;
        int slot = 0;
        Position position;
        readPoint(kObstaclePointsFile, row, 1, 2, layout_id, slot, position);
        if (!obstacle_slots[layout_id].emplace(slot, position).second) {
            throw WorldDataError(where(kObstaclePointsFile, row) + "slot " + std::to_string(slot) +
                                 " is defined twice for layout " + std::to_string(layout_id));
        }
    }

    for (const Row& row : readTable(directory, kHazardsFile, {"hazard_id", "layout_id", "x", "y", "damage", "name", "description"})) {
        int layout_id = toInt(kHazardsFile, row, 1, "layout_id");
        Hazard hazard;
        hazard.position = {toInt(kHazardsFile, row, 2, "x"), toInt(kHazardsFile, row, 3, "y")};
        hazard.damage = toInt(kHazardsFile, row, 4, "damage");
        hazard.name = row.fields[5];
        hazard.description = row.fields[6];
        auto layout = data.layouts.find(layout_id);
        if (layout == data.layouts.end()) {
            throw WorldDataError(where(kHazardsFile, row) + "layout " + std::to_string(layout_id) + " does not exist");
        }
        if (!layout->second.layout.contains(hazard.position)) {
            throw WorldDataError(where(kHazardsFile, row) + "point " + text(hazard.position) + " is outside layout " +
                                 std::to_string(layout_id));
        }
        if (hazard.damage < 1 || hazard.name.empty()) {
            throw WorldDataError(where(kHazardsFile, row) + "a hazard needs a name and damage of 1 or more");
        }
        layout->second.layout.hazards.push_back(hazard);
    }

    for (auto& [id, record] : data.layouts) {
        placeSlots(ei_slots[id], record.layout.ei_spawns, kSpawnPointsFile, id, "EI");
        placeSlots(r_slots[id], record.layout.r_spawns, kSpawnPointsFile, id, "R");
        placeSlots(obstacle_slots[id], record.layout.obstacle_points, kObstaclePointsFile, id, "obstacle");
    }

    for (const Row& row : readTable(directory, kExitsFile, {"exit_id", "from_room_id", "to_room_id", "direction", "description"})) {
        ExitRecord exit;
        exit.id = toInt(kExitsFile, row, 0, "exit_id");
        exit.from_room_id = toInt(kExitsFile, row, 1, "from_room_id");
        exit.to_room_id = toInt(kExitsFile, row, 2, "to_room_id");
        exit.direction = row.fields[3];
        exit.description = row.fields[4];
        data.exits.push_back(exit);
    }

    for (const Row& row : readTable(directory, kObstaclesFile, {"obstacle_type", "symbol", "name", "description"})) {
        ObstacleRecord obstacle;
        obstacle.type = toInt(kObstaclesFile, row, 0, "obstacle_type");
        const std::string& symbol = row.fields[1];
        obstacle.name = row.fields[2];
        obstacle.description = row.fields[3];
        if (!Obstacle::isValidType(obstacle.type)) {
            throw WorldDataError(where(kObstaclesFile, row) + "obstacle_type must be 1-10");
        }
        if (symbol.size() != 1 || symbol[0] <= ' ' || symbol[0] >= 127 || std::strchr(kReservedSymbols, symbol[0])) {
            throw WorldDataError(where(kObstaclesFile, row) + "symbol must be one visible character other than " +
                                 kReservedSymbols + ", found '" + symbol + "'");
        }
        obstacle.symbol = symbol[0];
        if (obstacle.name.empty()) {
            throw WorldDataError(where(kObstaclesFile, row) + "name must not be empty");
        }
        for (const auto& [type, other] : data.obstacles) {
            if (other.symbol == obstacle.symbol) {
                throw WorldDataError(where(kObstaclesFile, row) + "symbol '" + symbol + "' is already used by " + other.name);
            }
        }
        if (!data.obstacles.emplace(obstacle.type, obstacle).second) {
            throw WorldDataError(where(kObstaclesFile, row) + "obstacle type " + std::to_string(obstacle.type) + " is defined twice");
        }
    }

    for (const Row& row : readTable(directory, kEquipmentFile, {"equipment_id", "kind", "slot", "name", "description"})) {
        EquipmentRecord equipment;
        equipment.id = toInt(kEquipmentFile, row, 0, "equipment_id");
        equipment.kind = toKind(kEquipmentFile, row, 1);
        equipment.slot = toInt(kEquipmentFile, row, 2, "slot");
        equipment.name = row.fields[3];
        equipment.description = row.fields[4];
        auto& by_slot = equipment.kind == EquipmentKind::EmotionalIntelligence ? data.ei_equipment : data.r_equipment;
        if (!by_slot.emplace(equipment.slot, equipment).second) {
            throw WorldDataError(where(kEquipmentFile, row) + label(equipment.kind) + " slot " +
                                 std::to_string(equipment.slot) + " is defined twice");
        }
    }

    data.validate();
    return data;
}

void WorldData::validate() const {
    if (rooms.empty()) {
        throw WorldDataError(std::string(kRoomsFile) + ": there must be at least one room");
    }

    for (const auto& [id, record] : layouts) {
        const RoomLayout& layout = record.layout;
        std::string name = std::string(kLayoutsFile) + ": layout " + std::to_string(id) + " ";
        if (layout.width < 1 || layout.height < 1) {
            throw WorldDataError(name + "must be at least 1x1");
        }
        if (!layout.contains(layout.door) || !layout.contains(layout.entry)) {
            throw WorldDataError(name + "has its door or entry outside the grid");
        }
        if (layout.door == layout.entry) {
            throw WorldDataError(name + "has its door and entry on the same tile");
        }
        // Every point needs its own tile, away from the door and the entry.
        std::set<std::pair<int, int>> used = {{layout.door.x, layout.door.y}, {layout.entry.x, layout.entry.y}};
        auto claim = [&](const std::vector<Position>& points, const char* file, const char* what) {
            for (const Position& position : points) {
                if (!used.insert({position.x, position.y}).second) {
                    throw WorldDataError(std::string(file) + ": layout " + std::to_string(id) + " has " + what + " " +
                                         text(position) + " on the door, the entry or another point");
                }
            }
        };
        claim(layout.ei_spawns, kSpawnPointsFile, "spawn point");
        claim(layout.r_spawns, kSpawnPointsFile, "spawn point");
        claim(layout.obstacle_points, kObstaclePointsFile, "obstacle point");
        std::vector<Position> hazard_points;
        for (const Hazard& hazard : layout.hazards) {
            hazard_points.push_back(hazard.position);
        }
        claim(hazard_points, kHazardsFile, "hazard");
    }

    for (const RoomRecord& room : rooms) {
        std::string name = std::string(kRoomsFile) + ": room " + std::to_string(room.id) + " ";
        if (room.name.empty()) {
            throw WorldDataError(name + "has no name");
        }
        if (room.obstacle_types.empty()) {
            throw WorldDataError(name + "has no obstacles in " + kRoomObstaclesFile);
        }
        auto found = layouts.find(room.layout_id);
        if (found == layouts.end()) {
            throw WorldDataError(name + "uses layout " + std::to_string(room.layout_id) + ", which is not in " + kLayoutsFile);
        }
        const RoomLayout& layout = found->second.layout;
        if (room.obstacle_types.size() - 1 > layout.obstacle_points.size()) {
            throw WorldDataError(name + "has " + std::to_string(room.obstacle_types.size()) + " obstacles, but layout " +
                                 std::to_string(room.layout_id) + " has room for " +
                                 std::to_string(layout.obstacle_points.size() + 1) +
                                 " (its obstacle points plus the door)");
        }
        for (int type : room.obstacle_types) {
            if (!obstacles.count(type)) {
                throw WorldDataError(name + "uses obstacle type " + std::to_string(type) + ", which is not in " + kObstaclesFile);
            }
            int need_ei = Obstacle::requiredEmotionalIntelligenceFor(type);
            int need_r = Obstacle::requiredResilienceFor(type);
            if (need_ei > static_cast<int>(layout.ei_spawns.size()) || need_r > static_cast<int>(layout.r_spawns.size())) {
                throw WorldDataError(name + "has an obstacle needing " + std::to_string(need_ei) + " EI and " +
                                     std::to_string(need_r) + " R spawn points, but layout " +
                                     std::to_string(room.layout_id) + " has " + std::to_string(layout.ei_spawns.size()) +
                                     " and " + std::to_string(layout.r_spawns.size()));
            }
            for (int slot = 1; slot <= need_ei; ++slot) {
                equipmentFor(EquipmentKind::EmotionalIntelligence, slot);
            }
            for (int slot = 1; slot <= need_r; ++slot) {
                equipmentFor(EquipmentKind::Resilience, slot);
            }
        }
    }

    std::set<std::pair<int, std::string>> seen;
    int room_count = static_cast<int>(rooms.size());
    for (const ExitRecord& exit : exits) {
        std::string name = std::string(kExitsFile) + ": exit " + std::to_string(exit.id) + " ";
        if (exit.from_room_id < 1 || exit.from_room_id > room_count || exit.to_room_id < 1 || exit.to_room_id > room_count) {
            throw WorldDataError(name + "connects a room that does not exist");
        }
        if (exit.direction != "north" && exit.direction != "south") {
            throw WorldDataError(name + "has direction '" + exit.direction +
                                 "'; rooms have a north door and a south entry, so use north or south");
        }
        if (!seen.insert({exit.from_room_id, exit.direction}).second) {
            throw WorldDataError(name + "gives room " + std::to_string(exit.from_room_id) + " a second " +
                                 exit.direction + " exit");
        }
    }
}

const EquipmentRecord& WorldData::equipmentFor(EquipmentKind kind, int slot) const {
    const auto& by_slot = kind == EquipmentKind::EmotionalIntelligence ? ei_equipment : r_equipment;
    auto found = by_slot.find(slot);
    if (found == by_slot.end()) {
        throw WorldDataError(std::string(kEquipmentFile) + ": no " + label(kind) + " equipment for slot " +
                             std::to_string(slot));
    }
    return found->second;
}
