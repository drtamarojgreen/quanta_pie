#include "../Game.h"
#include <iostream>

volatile sig_atomic_t g_signal_received = 0;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <path_to_data_directory>" << std::endl;
        std::cout << "The directory must hold the gauntlet_*.csv tables, like sql/." << std::endl;
        return 1;
    }

    try {
        Game game(argv[1]);
        game.start();
    } catch (const ConfigError& error) {
        std::cerr << "Could not read " << GameConfig::kDefaultFile << ": " << error.what() << std::endl;
        return 1;
    } catch (const WorldDataError& error) {
        std::cerr << "Could not load the game world: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}
