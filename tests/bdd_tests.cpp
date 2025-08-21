#include "TestRunner.h"
#include "../src/Room.h"
#include "../src/players/Player.h"
#include "../src/objects/Character.h"
#include <vector>
#include <string>

// BDD-style test for player movement
bool testPlayerMovement() {
    // GIVEN a player and two connected rooms
    Room room1("Starting Room");
    Room room2("Next Room");
    room1.addExit("north", &room2);
    room2.addExit("south", &room1);

    Player player(0, "Test Player", "2024-01-01", &room1);

    // WHEN the player moves to the next room
    player.move("north");

    // THEN the player should be in the new room
    ASSERT_EQ(player.getCurrentRoom(), &room2);

    // WHEN the player moves back
    player.move("south");

    // THEN the player should be back in the starting room
    ASSERT_EQ(player.getCurrentRoom(), &room1);

    return true;
}

// BDD-style test for encountering a character
bool testEncounterCharacter() {
    // GIVEN a room with a character in it
    Room room("A room with a character");
    Character character("NPC", "A mysterious non-player character.", "Hello there.");
    room.addCharacter(&character);
    Player player(0, "Test Player", "2024-01-01", &room);

    // WHEN the player is in the room
    // (No action needed, the check is on the state)

    // THEN the player should be able to see the character
    const auto& characters_in_room = player.getCurrentRoom()->getCharacters();
    ASSERT_EQ(characters_in_room.size(), 1);
    ASSERT_EQ(characters_in_room[0]->getName(), "NPC");

    return true;
}


// Function to register all BDD tests with the runner
void registerBddTests(TestRunner& runner) {
    runner.addTest("testPlayerMovement", testPlayerMovement);
    runner.addTest("testEncounterCharacter", testEncounterCharacter);
}
