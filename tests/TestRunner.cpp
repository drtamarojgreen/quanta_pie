#include "TestRunner.h"

void TestRunner::addTest(const std::string& name, TestCase test) {
    tests.push_back({name, test});
}

void TestRunner::runTests() {
    int passed = 0;
    int failed = 0;

    std::cout << "Running " << tests.size() << " tests..." << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    for (const auto& t : tests) {
        std::cout << "Running test: " << t.name << "..." << std::endl;
        bool result = t.test();
        if (result) {
            std::cout << "[  PASSED  ] " << t.name << std::endl;
            passed++;
        } else {
            std::cout << "[  FAILED  ] " << t.name << std::endl;
            failed++;
        }
        std::cout << "----------------------------------------" << std::endl;
    }

    std::cout << "Test run finished." << std::endl;
    std::cout << "Passed: " << passed << ", Failed: " << failed << std::endl;
}
