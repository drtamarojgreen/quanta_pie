#include "TestRunner.h"
#include <csignal>

// Global flag for signal handling
volatile sig_atomic_t g_signal_received = 0;

// Forward declarations for test registration functions
void registerUnitTests(TestRunner& runner);
void registerBddTests(TestRunner& runner);
void registerEnvironmentTests(TestRunner& runner);
void registerGauntletTests(TestRunner& runner);
void registerWorldDataTests(TestRunner& runner);
void registerGameTests(TestRunner& runner);

int main() {
    TestRunner runner;

    // Register all tests
    registerUnitTests(runner);
    registerBddTests(runner);
    registerEnvironmentTests(runner);
    registerGauntletTests(runner);
    registerWorldDataTests(runner);
    registerGameTests(runner);

    // Run all tests; a non-zero exit code signals failures
    int failed = runner.runTests();

    return failed == 0 ? 0 : 1;
}
