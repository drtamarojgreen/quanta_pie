#include "TestRunner.h"

// Forward declarations for test registration functions
void registerUnitTests(TestRunner& runner);
void registerBddTests(TestRunner& runner);

int main() {
    TestRunner runner;

    // Register all tests
    registerUnitTests(runner);
    registerBddTests(runner);

    // Run all tests
    runner.runTests();

    return 0;
}
