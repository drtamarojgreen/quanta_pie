#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include <iostream>
#include <string>
#include <vector>
#include <functional>

// A simple assertion function
#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        std::cerr << "Assertion failed: " #condition ", file " << __FILE__ << ", line " << __LINE__ << std::endl; \
        return false; \
    }

// A more general assertion with a message
#define ASSERT_EQ(val1, val2) \
    if (!((val1) == (val2))) { \
        std::cerr << "Assertion failed: " #val1 " == " #val2 " (" << (val1) << " vs " << (val2) << "), file " << __FILE__ << ", line " << __LINE__ << std::endl; \
        return false; \
    }


class TestRunner {
public:
    // A test case is a function that returns true on success and false on failure
    using TestCase = std::function<bool()>;

    // Register a test case with a given name
    void addTest(const std::string& name, TestCase test);

    // Run all registered test cases
    void runTests();

private:
    struct Test {
        std::string name;
        TestCase test;
    };

    std::vector<Test> tests;
};

#endif // TEST_RUNNER_H
