#include "Game.h"
#include <iostream> // For std::cout, std::endl
#include <csignal>  // For std::signal, SIGINT

// Global flag to indicate if a signal has been received
volatile sig_atomic_t g_signal_received = 0;

// Signal handler function
void signal_handler(int signal) {
    if (signal == SIGINT) {
        g_signal_received = 1;
        std::cout << "Ctrl+C detected. Attempting graceful shutdown..." << std::endl;
    }
}

int main() {
    // Register the signal handler for SIGINT (Ctrl+C)
    std::signal(SIGINT, signal_handler);

    Game game;
    game.start();

    return 0;
}
