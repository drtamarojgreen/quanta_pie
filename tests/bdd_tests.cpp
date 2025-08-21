#include "TestRunner.h"
#include "../src/Room.h"
#include "../src/players/Player.h"
#include "../src/objects/Character.h"
#include "../src/objects/Tool.h"
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
    player.setCurrentRoom(room1.getExit("north"));

    // THEN the player should be in the new room
    ASSERT_EQ(player.getCurrentRoom(), &room2);

    // WHEN the player moves back
    player.setCurrentRoom(room2.getExit("south"));

    // THEN the player should be back in the starting room
    ASSERT_EQ(player.getCurrentRoom(), &room1);

    return true;
}

// BDD-style test for encountering a character
bool testEncounterCharacter() {
    // GIVEN a room with a character in it
    Room room("A room with a character");
    Character character(1, "NPC", "A mysterious non-player character.", 1, "Hello there.");
    room.addObject(&character);
    Player player(0, "Test Player", "2024-01-01", &room);

    // WHEN the player is in the room
    // (No action needed, the check is on the state)

    // THEN the player should be able to see the character
    const auto& objects_in_room = player.getCurrentRoom()->getObjects();
    ASSERT_EQ(objects_in_room.size(), 1);
    Character* found_character = dynamic_cast<Character*>(objects_in_room[0]);
    ASSERT_TRUE(found_character != nullptr);
    ASSERT_EQ(found_character->getName(), "NPC");

    return true;
}


// BDD-style test for player picking up a tool
bool testPlayerPicksUpTool() {
    // GIVEN a room with a tool in it and a player
    Room room("A dusty workshop");
    Tool tool(1, "Wrench", "A rusty wrench.", 1);
    room.addObject(&tool);
    Player player(0, "Test Player", "2024-01-01", &room);

    // WHEN the player picks up the tool
    player.takeTool(&tool);

    // THEN the tool should be in the player's inventory
    const auto& inventory = player.getTools();
    ASSERT_EQ(inventory.size(), 1);
    ASSERT_EQ(inventory[0]->getName(), "Wrench");

    // AND the tool should be removed from the room
    const auto& objects_in_room = room.getObjects();
    ASSERT_EQ(objects_in_room.size(), 0);

    return true;
}

// BDD-style test for player inventory
bool testPlayerInventory() {
    // GIVEN a player with a tool in their inventory
    Player player(0, "Test Player", "2024-01-01", nullptr);
    Tool tool(1, "Screwdriver", "A magnetic screwdriver.", 1);
    player.takeTool(&tool);

    // WHEN we check the player's inventory
    const auto& inventory = player.getTools();

    // THEN the inventory should contain the tool
    ASSERT_EQ(inventory.size(), 1);
    ASSERT_EQ(inventory[0]->getName(), "Screwdriver");

    return true;
}


// Function to register all BDD tests with the runner
void registerBddTests(TestRunner& runner) {
    runner.addTest("testPlayerMovement", testPlayerMovement);
    runner.addTest("testEncounterCharacter", testEncounterCharacter);
    runner.addTest("testPlayerPicksUpTool", testPlayerPicksUpTool);
    runner.addTest("testPlayerInventory", testPlayerInventory);
}
