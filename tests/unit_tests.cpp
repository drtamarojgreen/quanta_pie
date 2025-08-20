// Comprehensive unit tests for the Quanta_Pie game classes.
// Note: This suite is designed to be run without external frameworks.
// Per instructions, this code is submitted without being compiled or executed.

#include <iostream>
#include <string>
#include <cassert>
#include <functional>

// Include the classes to be tested
// Note: The paths assume the compiler is run from the root directory, e.g., g++ -I. tests/unit_tests.cpp src/Room.cpp ...
#include "src/Room.h"
#include "src/Player.h"

// A simple test runner function to provide clear output
void run_test(const std::string& test_name, const std::function<void()>& test_func) {
    try {
        test_func();
        std::cout << "[PASS] " << test_name << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[FAIL] " << test_name << " - Exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[FAIL] " << test_name << " - Unknown exception" << std::endl;
    }
}

// --- Test Cases for Room Class ---

void testRoomCreation() {
    Room test_room("A test room with a specific description.");
    assert(test_room.getDescription() == "A test room with a specific description.");
}

void testAddAndGetExit() {
    Room room1("Room 1");
    Room room2("Room 2");
    room1.addExit("north", &room2);
    assert(room1.getExit("north") == &room2);
}

void testGetNonExistentExit() {
    Room test_room("A lonely room.");
    assert(test_room.getExit("south") == nullptr);
}

void testAddMultipleExits() {
    Room center_room("Center Room");
    Room north_room("North Room");
    Room south_room("South Room");

    center_room.addExit("north", &north_room);
    center_room.addExit("south", &south_room);

    assert(center_room.getExit("north") == &north_room);
    assert(center_room.getExit("south") == &south_room);
    assert(center_room.getExit("east") == nullptr);
}

void testOverwriteExit() {
    Room room1("Room 1");
    Room room2("Room 2");
    Room room3("Room 3");

    room1.addExit("north", &room2);
    assert(room1.getExit("north") == &room2);

    room1.addExit("north", &room3);
    assert(room1.getExit("north") == &room3);
}


// --- Test Cases for Player Class ---

void testPlayerCreation() {
    Room starting_room("The starting room.");
    Player player(&starting_room);
    assert(player.getCurrentRoom() == &starting_room);
}

void testSetCurrentRoom() {
    Room room1("Room 1");
    Room room2("Room 2");
    Player player(&room1);

    player.setCurrentRoom(&room2);
    assert(player.getCurrentRoom() == &room2);
}

void testGetRepresentation() {
    Room room("A room");
    Player player(&room);
    // Assuming the representation is "@" as per typical text adventures
    assert(player.getRepresentation() == "@");
}


// --- Main Test Runner ---

int main() {
    std::cout << "--- Running Unit Tests for Quanta_Pie ---" << std::endl;

    // Room tests
    run_test("testRoomCreation", testRoomCreation);
    run_test("testAddAndGetExit", testAddAndGetExit);
    run_test("testGetNonExistentExit", testGetNonExistentExit);
    run_test("testAddMultipleExits", testAddMultipleExits);
    run_test("testOverwriteExit", testOverwriteExit);

    // Player tests
    run_test("testPlayerCreation", testPlayerCreation);
    run_test("testSetCurrentRoom", testSetCurrentRoom);
    run_test("testGetRepresentation", testGetRepresentation);

    std::cout << "--- All tests complete ---" << std::endl;

    return 0;
}
