#include "TestRunner.h"
#include "TestData.h"
#include "../src/CSVParser.h"
#include "../src/environments/WorldData.h"
#include "../src/environments/Environment.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>

namespace {

// Loads a world and returns the WorldDataError message, or "" if it loaded.
std::string loadError(const std::string& directory) {
    try {
        WorldData::loadFromDirectory(directory);
    } catch (const WorldDataError& error) {
        return error.what();
    }
    return "";
}

bool contains(const std::string& text, const std::string& part) {
    if (text.find(part) == std::string::npos) {
        std::cerr << "Expected \"" << part << "\" in \"" << text << "\"" << std::endl;
        return false;
    }
    return true;
}

} // namespace

// Test case: quoted fields, escaped quotes and Windows line endings
bool testCSVParser_QuotesAndLineEndings() {
    std::vector<std::string> fields = CSVParser::split("1,\"a, b\",\"say \"\"hi\"\"\",", ',');
    ASSERT_EQ(fields.size(), 4u);
    ASSERT_EQ(fields[1], "a, b");
    ASSERT_EQ(fields[2], "say \"hi\"");
    ASSERT_EQ(fields[3], "");

    std::string directory = writeWorld("csv", {{"table.csv", "id,name\r\n1,\"x\"\r\n"}});
    std::vector<std::vector<std::string>> rows = CSVParser::readCSV(directory + "/table.csv");
    ASSERT_EQ(rows.size(), 2u);
    ASSERT_EQ(rows[0][1], "name");
    ASSERT_EQ(rows[1][1], "x");
    ASSERT_TRUE(CSVParser::readCSV(directory + "/missing.csv").empty());
    std::filesystem::remove_all(directory);
    return true;
}

// Test case: the shipped tables in sql/ describe 100 rooms of increasing difficulty
bool testWorldData_LoadsShippedTables() {
    WorldData data = WorldData::loadFromDirectory(kSqlDir);
    ASSERT_EQ(data.rooms.size(), 100u);

    std::set<std::string> room_names;
    size_t previous_count = 0;
    for (size_t i = 0; i < data.rooms.size(); ++i) {
        const RoomRecord& room = data.rooms[i];
        ASSERT_EQ(room.id, static_cast<int>(i) + 1);
        ASSERT_TRUE(!room.description.empty());
        ASSERT_TRUE(room.obstacle_types.size() >= previous_count);
        previous_count = room.obstacle_types.size();
        int cost = 0;
        for (int type : room.obstacle_types) {
            cost += type; // A type-t obstacle needs t EI+R in total
        }
        ASSERT_EQ(cost, room.id);
        room_names.insert(room.name);
    }
    ASSERT_EQ(room_names.size(), 100u);
    ASSERT_TRUE(data.rooms[0].obstacle_types == std::vector<int>{1});
    ASSERT_TRUE(data.rooms[56].obstacle_types == std::vector<int>({7, 7, 7, 7, 7, 7, 7, 8}));
    ASSERT_TRUE(data.rooms[99].obstacle_types == std::vector<int>(10, 10));
    ASSERT_EQ(data.rooms[0].name, "Hall of Echoes I");
    ASSERT_EQ(data.rooms[99].name, "Summit of Embers X");

    std::set<std::string> obstacle_names;
    std::set<char> symbols;
    for (const auto& [type, obstacle] : data.obstacles) {
        obstacle_names.insert(obstacle.name);
        symbols.insert(obstacle.symbol);
    }
    ASSERT_EQ(obstacle_names.size(), 10u); // Ten distinct obstacles...
    ASSERT_EQ(symbols.size(), 10u);        // ...each drawn with its own symbol
    ASSERT_EQ(data.obstacles.at(5).symbol, '&');
    ASSERT_EQ(data.obstacles.at(1).name, "Self-Doubt");
    ASSERT_EQ(data.obstacles.at(10).name, "Burnout");

    ASSERT_EQ(data.layouts.size(), 1u);
    const RoomLayout& layout = data.layouts.at(1).layout;
    ASSERT_EQ(layout.width, 13);
    ASSERT_EQ(layout.height, 7);
    ASSERT_TRUE(layout.door == Position(6, 0));
    ASSERT_TRUE(layout.entry == Position(6, 6));
    ASSERT_EQ(layout.ei_spawns.size(), 5u);
    ASSERT_EQ(layout.r_spawns.size(), 5u);
    ASSERT_EQ(layout.obstacle_points.size(), 9u);
    ASSERT_EQ(layout.hazards.size(), 3u);
    ASSERT_TRUE(layout.hazardAt({6, 5}) != nullptr);
    ASSERT_EQ(layout.hazardAt({6, 5})->damage, 3);
    ASSERT_EQ(layout.hazardAt({6, 5})->name, "Rumination Loop");
    ASSERT_TRUE(layout.hazardAt({6, 6}) == nullptr);
    ASSERT_TRUE(layout.obstacle_points[0] == Position(5, 0));
    ASSERT_EQ(data.ei_equipment.size(), 5u);
    ASSERT_EQ(data.r_equipment.size(), 5u);
    ASSERT_EQ(data.equipmentFor(EquipmentKind::Resilience, 5).name, "Ember of Purpose");

    // Rooms are chained north and south
    ASSERT_EQ(data.exits.size(), 198u);
    std::set<std::pair<int, int>> north;
    std::set<std::pair<int, int>> south;
    for (const ExitRecord& exit : data.exits) {
        (exit.direction == "north" ? north : south).insert({exit.from_room_id, exit.to_room_id});
    }
    for (int number = 1; number < 100; ++number) {
        ASSERT_TRUE(north.count({number, number + 1}) == 1);
        ASSERT_TRUE(south.count({number + 1, number}) == 1);
    }
    return true;
}

// Test case: a custom world's rooms and spawn points come entirely from its tables
bool testWorldData_BuildsCustomWorld() {
    std::string directory = writeWorld("custom", customWorldTables());
    Environment env(directory);
    ASSERT_EQ(env.getRoomCount(), 3);
    ASSERT_EQ(env.getRoomName(3), "Roof");
    ASSERT_EQ(env.getRoom(1)->getDescription(), "A small, bright foyer.");
    ASSERT_EQ(env.getRoom(3)->getDescription(), "A windy roof, under a \"huge\" sky.");
    // Room 2's obstacles were listed out of order; they are placed by sequence
    const std::vector<Obstacle*>& study = env.getObstacles(2);
    ASSERT_EQ(study.size(), 2u);
    ASSERT_EQ(study[0]->getLabel(), "N Nerves");
    ASSERT_TRUE(study[0]->getPosition() == Position(2, 0)); // Obstacle point 1
    ASSERT_EQ(study[1]->getLabel(), "C Crossed Wires");
    ASSERT_TRUE(study[1]->getPosition() == Position(3, 0)); // The door
    ASSERT_EQ(env.getRoom(1)->getExit("north"), env.getRoom(2));
    ASSERT_EQ(env.getRoom(1)->getExit("south"), nullptr);

    // Room 3's second obstacle (type 6) fills all five EI slots and the one R slot of the compact layout
    std::set<std::pair<int, int>> tiles;
    for (const Equipment* piece : env.getEquipmentFor(env.getObstacles(3)[1])) {
        tiles.insert({piece->getPosition().x, piece->getPosition().y});
    }
    std::set<std::pair<int, int>> expected = {{1, 1}, {2, 1}, {4, 1}, {5, 1}, {1, 2}, {5, 2}};
    ASSERT_TRUE(tiles == expected);
    std::filesystem::remove_all(directory);
    return true;
}

// Test case: missing or inconsistent tables are rejected with the file and problem named
bool testWorldData_RejectsBadData() {
    struct Case {
        const char* file;
        std::string contents; // Empty means delete the file
        const char* expected;
    };
    auto tables = customWorldTables();
    const std::string rooms_header = "room_id,name,description,layout_id\n";
    const std::string sequence_header = "room_obstacle_id,room_id,sequence,obstacle_type\n";
    const std::string spawn_header = "spawn_point_id,layout_id,kind,slot,x,y\n";
    const std::string points_header = "obstacle_point_id,layout_id,slot,x,y\n";
    const std::string obstacles_header = "obstacle_type,symbol,name,description\n";
    const std::vector<Case> cases = {
        {"gauntlet_rooms.csv", "", "gauntlet_rooms.csv: missing or empty"},
        {"gauntlet_rooms.csv", "id,name\n1,Foyer\n", "gauntlet_rooms.csv line 1: header must be room_id,name"},
        {"gauntlet_rooms.csv", rooms_header + "1,Foyer,x\n", "gauntlet_rooms.csv line 2: expected 4 fields, found 3"},
        {"gauntlet_rooms.csv", rooms_header + "1,Foyer,x,7\n3,Study,x,7\n", "room ids must run 1, 2, 3"},
        {"gauntlet_rooms.csv", rooms_header + "1,Foyer,x,9\n2,Study,x,7\n3,Roof,x,7\n", "room 1 uses layout 9, which is not in gauntlet_layouts.csv"},
        {"gauntlet_room_obstacles.csv", sequence_header + "1,1,1,11\n", "gauntlet_room_obstacles.csv line 2: obstacle_type must be 1-10"},
        {"gauntlet_room_obstacles.csv", sequence_header + "1,1,1,one\n", "line 2: obstacle_type must be a whole number, found 'one'"},
        {"gauntlet_room_obstacles.csv", sequence_header + "1,4,1,1\n", "line 2: room 4 does not exist"},
        {"gauntlet_room_obstacles.csv", sequence_header + "1,1,1,1\n2,1,1,1\n", "room 1 already has an obstacle at sequence 1"},
        {"gauntlet_room_obstacles.csv", sequence_header + "1,1,1,1\n2,1,3,1\n", "room 1 is missing sequence 2"},
        {"gauntlet_room_obstacles.csv", sequence_header + "1,1,1,1\n2,3,1,1\n", "room 2 has no obstacles in gauntlet_room_obstacles.csv"},
        {"gauntlet_room_obstacles.csv", sequence_header + "1,1,1,1\n2,2,1,2\n3,3,1,1\n", "room 2 uses obstacle type 2, which is not in gauntlet_obstacles.csv"},
        {"gauntlet_room_obstacles.csv", sequence_header + "1,1,1,1\n2,1,2,1\n3,1,3,1\n4,1,4,1\n5,2,1,1\n6,3,1,1\n",
         "room 1 has 4 obstacles, but layout 7 has room for 3 (its obstacle points plus the door)"},
        {"gauntlet_obstacle_points.csv", points_header + "1,7,1,1,1\n", "has obstacle point (1,1) on the door, the entry or another point"},
        {"gauntlet_obstacle_points.csv", points_header + "1,7,2,2,0\n", "layout 7 is missing obstacle slot 1"},
        {"gauntlet_obstacles.csv", obstacles_header + "1,@,Nerves,x\n3,C,Crossed Wires,x\n6,W,Washout,x\n",
         "gauntlet_obstacles.csv line 2: symbol must be one visible character other than .@ER#+-|^v:, found '@'"},
        {"gauntlet_hazards.csv", "hazard_id,layout_id,x,y,damage,name,description\n1,7,1,1,3,Pit,x\n",
         "layout 7 has hazard (1,1) on the door, the entry or another point"},
        {"gauntlet_hazards.csv", "hazard_id,layout_id,x,y,damage,name,description\n1,7,0,3,0,Pit,x\n",
         "gauntlet_hazards.csv line 2: a hazard needs a name and damage of 1 or more"},
        {"gauntlet_hazards.csv", "hazard_id,layout_id,x,y,damage,name,description\n1,7,9,9,3,Pit,x\n",
         "gauntlet_hazards.csv line 2: point (9,9) is outside layout 7"},
        {"gauntlet_hazards.csv", "hazard_id,layout_id,x,y,damage,name,description\n1,8,0,3,3,Pit,x\n",
         "gauntlet_hazards.csv line 2: layout 8 does not exist"},
        {"gauntlet_obstacles.csv", obstacles_header + "1,NN,Nerves,x\n", "symbol must be one visible character"},
        {"gauntlet_obstacles.csv", obstacles_header + "1,N,Nerves,x\n3,N,Crossed Wires,x\n", "line 3: symbol 'N' is already used by Nerves"},
        {"gauntlet_spawn_points.csv", spawn_header + "1,7,EI,1,9,1\n", "line 2: point (9,1) is outside layout 7"},
        {"gauntlet_spawn_points.csv", spawn_header + "1,7,EI,1,3,0\n", "has spawn point (3,0) on the door"},
        {"gauntlet_spawn_points.csv", spawn_header + "1,7,EI,1,1,1\n2,7,EI,3,2,1\n", "layout 7 is missing EI slot 2"},
        {"gauntlet_spawn_points.csv", spawn_header + "1,7,XP,1,1,1\n", "kind must be EI or R, found 'XP'"},
        {"gauntlet_spawn_points.csv", spawn_header + "1,7,EI,1,1,1\n2,7,EI,2,2,1\n3,7,EI,3,4,1\n4,7,R,1,5,2\n",
         "room 3 has an obstacle needing 5 EI and 1 R spawn points, but layout 7 has 3 and 1"},
        {"gauntlet_exits.csv", "exit_id,from_room_id,to_room_id,direction,description\n1,1,2,east,x\n", "use north or south"},
        {"gauntlet_exits.csv", "exit_id,from_room_id,to_room_id,direction,description\n1,1,4,north,x\n", "connects a room that does not exist"},
        {"gauntlet_equipment.csv", "equipment_id,kind,slot,name,description\n1,EI,1,Pause,x\n", "no EI equipment for slot 2"},
        {"gauntlet_layouts.csv", "layout_id,name,width,height,door_x,door_y,entry_x,entry_y\n7,Compact,7,5,3,0,3,0\n",
         "layout 7 has its door and entry on the same tile"},
    };
    for (const Case& test_case : cases) {
        auto broken = tables;
        broken[test_case.file] = test_case.contents;
        std::string directory = writeWorld("broken", broken);
        if (test_case.contents.empty()) {
            std::filesystem::remove(std::filesystem::path(directory) / test_case.file);
        }
        std::string error = loadError(directory);
        std::filesystem::remove_all(directory);
        ASSERT_TRUE(contains(error, test_case.expected));
    }
    ASSERT_TRUE(contains(loadError("/nonexistent/quantapie"), "gauntlet_rooms.csv: missing or empty"));
    return true;
}

// Function to register all world data tests with the runner
void registerWorldDataTests(TestRunner& runner) {
    runner.addTest("testCSVParser_QuotesAndLineEndings", testCSVParser_QuotesAndLineEndings);
    runner.addTest("testWorldData_LoadsShippedTables", testWorldData_LoadsShippedTables);
    runner.addTest("testWorldData_BuildsCustomWorld", testWorldData_BuildsCustomWorld);
    runner.addTest("testWorldData_RejectsBadData", testWorldData_RejectsBadData);
}
