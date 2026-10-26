#include "Game.h"
#include <iostream> // For std::cout, std::endl
#include <csignal>  // For std::signal, SIGINT

// Global flag to indicate if a signal has been received
volatile sig_atomic_t g_signal_received = 0;

// Signal handler function
void signal_handler(int signal) {
    if (signal == SIGINT) {
        g_signal_received = 1;
    }
}

int main() {
    // Register the signal handler for SIGINT (Ctrl+C)
    std::signal(SIGINT, signal_handler);

    try {
        Game game; // Reads config.txt if present, then loads the world from the CSV files in sql/
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
