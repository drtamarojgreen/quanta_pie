#include "../Game.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <path_to_sql_file>" << std::endl;
        return 1;
    }

    Game game(argv[1]);
    game.start();

    return 0;
}
