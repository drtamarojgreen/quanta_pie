#include "Map.h"

Map::Map() {
    worldMap = {
        "####################",
        "#..................#",
        "#..#..............##",
        "#..#...............#",
        "#..#####....########",
        "#........#.........#",
        "#........#.........#",
        "#........#.........#",
        "##########.........#",
        "#..................#",
        "#..................#",
        "####################"
    };
    width = worldMap[0].size();
    height = worldMap.size();
}

char Map::getTile(int x, int y) const {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        return worldMap[y][x];
    }
    return '#'; // Return a wall for out-of-bounds access
}
