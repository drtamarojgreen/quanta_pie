#include "TestRunner.h"
#include "../src/Room.h"
#include "../src/objects/Challenge.h"
#include "../src/objects/Character.h"
#include "../src/players/Player.h"
#include "../src/Score.h"
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


// Test case for Character class
bool testCharacter_Creation() {
    Character character("Gandalf", "A powerful wizard", "You shall not pass!");
    ASSERT_EQ(character.getName(), "Gandalf");
    ASSERT_EQ(character.getDescription(), "A powerful wizard");
    ASSERT_EQ(character.getDialogue(), "You shall not pass!");
    return true;
}

// Test case for Player class
bool testPlayer_CreationAndScore() {
    Player player(1, "Aragorn", "2023-01-01", nullptr);
    ASSERT_EQ(player.getID(), 1);
    ASSERT_EQ(player.getName(), "Aragorn");
    ASSERT_EQ(player.getJoinDate(), "2023-01-01");
    ASSERT_EQ(player.getScore(), 0);
    player.incrementScore(10);
    ASSERT_EQ(player.getScore(), 10);
    return true;
}

// Test case for Player room navigation
bool testPlayer_RoomNavigation() {
    Room room1("Room 1");
    Room room2("Room 2");
    Player player(1, "Test Player", "2023-01-01", &room1);

    ASSERT_EQ(player.getCurrentRoom(), &room1);
    player.setCurrentRoom(&room2);
    ASSERT_EQ(player.getCurrentRoom(), &room2);
    return true;
}

// Test case for Score class
bool testScore_Creation() {
    Score score(1, 1, 1, 100);
    ASSERT_EQ(score.getPlayerID(), 1);
    ASSERT_EQ(score.getScoreValue(), 100);
    return true;
}


// Function to register all unit tests with the runner
void registerUnitTests(TestRunner& runner) {
    runner.addTest("testRoom_AddAndGetExit", testRoom_AddAndGetExit);
    runner.addTest("testChallenge_Creation", testChallenge_Creation);
    runner.addTest("testCharacter_Creation", testCharacter_Creation);
    runner.addTest("testPlayer_CreationAndScore", testPlayer_CreationAndScore);
    runner.addTest("testPlayer_RoomNavigation", testPlayer_RoomNavigation);
    runner.addTest("testScore_Creation", testScore_Creation);
}
