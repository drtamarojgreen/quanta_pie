#ifndef MAP_H
#define MAP_H

#include <vector>
#include <string>

class Map {
public:
    Map();
    char getTile(int x, int y) const;

private:
    std::vector<std::string> worldMap;
    int width;
    int height;
};

#endif // MAP_H
