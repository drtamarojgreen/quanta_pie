#ifndef TEST_DATA_H
#define TEST_DATA_H

#include <filesystem>
#include <fstream>
#include <map>
#include <string>

// The shipped world data; the Makefile passes its absolute path.
#ifndef QUANTAPIE_SQL_DIR
#define QUANTAPIE_SQL_DIR "../sql"
#endif
inline const std::string kSqlDir = QUANTAPIE_SQL_DIR;

/**
 * The tables of a small three-room world with its own 7x5 layout:
 *
 *   +---^---+
 *   |..1X2..|   X  last obstacle on the door; 1, 2 the other obstacle points
 *   |.EE.EE.|   EI slots 1-4
 *   |.E...R.|   EI slot 5, R slot 1
 *   |......:|   a Puddle hazard (4 damage)
 *   |...@...|
 *   +-------+
 *
 * Room 1 has one obstacle (type 1), room 2 two (types 1, 3) and room 3 two (types 3, 6),
 * drawn as N, C and W. Tests edit single tables to make it invalid.
 * The rooms table uses Windows line endings.
 */
inline std::map<std::string, std::string> customWorldTables() {
    return {
        {"gauntlet_rooms.csv",
         "room_id,name,description,layout_id\r\n"
         "1,Foyer,\"A small, bright foyer.\",7\r\n"
         "2,Study,A cluttered study.,7\r\n"
         "3,Roof,\"A windy roof, under a \"\"huge\"\" sky.\",7\r\n"},
        {"gauntlet_room_obstacles.csv",
         "room_obstacle_id,room_id,sequence,obstacle_type\n"
         "1,1,1,1\n"
         "2,2,2,3\n"
         "3,2,1,1\n"
         "4,3,1,3\n"
         "5,3,2,6\n"},
        {"gauntlet_layouts.csv",
         "layout_id,name,width,height,door_x,door_y,entry_x,entry_y\n"
         "7,Compact,7,5,3,0,3,4\n"},
        {"gauntlet_spawn_points.csv",
         "spawn_point_id,layout_id,kind,slot,x,y\n"
         "1,7,EI,1,1,1\n"
         "2,7,EI,2,2,1\n"
         "3,7,EI,3,4,1\n"
         "4,7,EI,4,5,1\n"
         "5,7,EI,5,1,2\n"
         "6,7,R,1,5,2\n"},
        {"gauntlet_obstacle_points.csv",
         "obstacle_point_id,layout_id,slot,x,y\n"
         "1,7,1,2,0\n"
         "2,7,2,4,0\n"},
        {"gauntlet_hazards.csv",
         "hazard_id,layout_id,x,y,damage,name,description\n"
         "1,7,6,3,4,Puddle,Cold and deeper than it looks.\n"},
        {"gauntlet_exits.csv",
         "exit_id,from_room_id,to_room_id,direction,description\n"
         "1,1,2,north,Stairs lead up to the study.\n"
         "2,2,1,south,Stairs lead down to the foyer.\n"
         "3,2,3,north,A ladder climbs to the roof.\n"
         "4,3,2,south,A ladder drops to the study.\n"},
        {"gauntlet_obstacles.csv",
         "obstacle_type,symbol,name,description\n"
         "1,N,Nerves,Your hands will not stop shaking.\n"
         "3,C,Crossed Wires,Someone heard something you never said.\n"
         "6,W,Washout,Rain has swept away the path.\n"},
        {"gauntlet_equipment.csv",
         "equipment_id,kind,slot,name,description\n"
         "1,EI,1,Pause,Notice the feeling.\n"
         "2,EI,2,Breath,Slow down.\n"
         "3,EI,3,Ear,Listen first.\n"
         "4,EI,4,Spark,Remember why.\n"
         "5,EI,5,Handshake,Meet halfway.\n"
         "6,R,1,Umbrella,Weather it.\n"},
    };
}

// Writes tables to a fresh directory under the system temp directory and returns its path.
inline std::string writeWorld(const std::string& name, const std::map<std::string, std::string>& tables) {
    std::filesystem::path directory = std::filesystem::temp_directory_path() / ("quantapie_world_" + name);
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);
    for (const auto& [file, contents] : tables) {
        std::ofstream(directory / file, std::ios::binary) << contents;
    }
    return directory.string();
}

#endif // TEST_DATA_H
