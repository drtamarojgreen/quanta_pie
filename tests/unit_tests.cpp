#include "TestRunner.h"
#include "../src/Room.h"
#include "../src/objects/Challenge.h"
#include <vector>
#include <string>

// Test case for Room class
bool testRoom_AddAndGetExit() {
    Room room("Test Room");
    Room north_room("North Room");

    room.addExit("north", &north_room);

    ASSERT_EQ(room.getExit("north"), &north_room);
    ASSERT_EQ(room.getExit("south"), nullptr);

    return true;
}

// Test case for Challenge class
bool testChallenge_Creation() {
    bool action1_called = false;
    bool action2_called = false;

    std::vector<CBTChoice> choices = {
        {"Choice 1", [&](){ action1_called = true; }},
        {"Choice 2", [&](){ action2_called = true; }}
    };

    Challenge challenge("Test Thought", choices);

    ASSERT_EQ(challenge.getThought(), "Test Thought");
    ASSERT_EQ(challenge.getChoices().size(), 2);
    ASSERT_EQ(challenge.getChoices()[0].description, "Choice 1");

    // "Run" the action to test it
    challenge.getChoices()[0].action();
    ASSERT_TRUE(action1_called);
    ASSERT_TRUE(!action2_called);

    return true;
}


// Function to register all unit tests with the runner
void registerUnitTests(TestRunner& runner) {
    runner.addTest("testRoom_AddAndGetExit", testRoom_AddAndGetExit);
    runner.addTest("testChallenge_Creation", testChallenge_Creation);
}
