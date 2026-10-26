#include "TestRunner.h"
#include "TestData.h"
#include "../src/Room.h"
#include "../src/players/Player.h"
#include "../src/players/PlayerStats.h"
#include "../src/objects/Equipment.h"
#include "../src/objects/Obstacle.h"
#include "../src/objects/Position.h"
#include "../src/environments/Environment.h"
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Test case for Position stepping and distance
bool testPosition_StepAndDistance() {
    Position origin(5, 5);
    ASSERT_TRUE(!Position().isValid());
    ASSERT_TRUE(origin.isValid());
    ASSERT_TRUE(origin.step(Direction::North) == Position(5, 4));
    ASSERT_TRUE(origin.step(Direction::SouthEast) == Position(6, 6));
    ASSERT_TRUE(origin.step(Direction::NorthWest) == Position(4, 4));
    ASSERT_EQ(origin.distanceTo(Position(6, 6)), 1); // Diagonal neighbours are adjacent
    ASSERT_EQ(origin.distanceTo(Position(8, 3)), 3);
    ASSERT_EQ(directionName(Direction::SouthWest), "southwest");
    return true;
}

// Test case for placing a RoomObject on the grid
bool testRoomObject_Position() {
    RoomObject object(1, "Lamp", "", 1);
    ASSERT_TRUE(!object.getPosition().isValid());
    object.setPosition({3, 4});
    ASSERT_TRUE(object.getPosition() == Position(3, 4));
    return true;
}

// Test case for Equipment class
bool testEquipment_Creation() {
    Equipment equipment(1, "Lens of Empathy", "See through their eyes.", 3, EquipmentKind::EmotionalIntelligence, 2);
    ASSERT_EQ(equipment.getName(), "Lens of Empathy");
    ASSERT_EQ(equipment.getInitialRoomId(), 3);
    ASSERT_EQ(equipment.getWeight(), 1);
    ASSERT_TRUE(equipment.getKind() == EquipmentKind::EmotionalIntelligence);
    ASSERT_EQ(equipment.getPoints(), 2);
    ASSERT_TRUE(!equipment.isConsumed());
    equipment.consume();
    ASSERT_TRUE(equipment.isConsumed());
    return true;
}

// Test case for the requirements of all ten obstacle types
bool testObstacle_RequirementsForAllTypes() {
    const int expected_ei[] = {1, 2, 3, 4, 5, 5, 5, 5, 5, 5};
    const int expected_r[]  = {0, 0, 0, 0, 0, 1, 2, 3, 4, 5};
    for (int type = 1; type <= 10; ++type) {
        Obstacle obstacle(type, type, 1, "Obstacle", "A test obstacle.");
        ASSERT_EQ(obstacle.getType(), type);
        ASSERT_EQ(obstacle.getName(), "Obstacle");
        ASSERT_EQ(obstacle.getLabel(), "X Obstacle"); // 'X' unless the data gives a symbol
        ASSERT_EQ(obstacle.getRequiredEmotionalIntelligence(), expected_ei[type - 1]);
        ASSERT_EQ(obstacle.getRequiredResilience(), expected_r[type - 1]);
    }
    return true;
}

// Test case for rejecting obstacle types outside 1-10
bool testObstacle_RejectsInvalidType() {
    for (int type : {0, 11, -1}) {
        bool threw = false;
        try {
            Obstacle obstacle(1, type, 1, "Invalid", "");
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        ASSERT_TRUE(threw);
    }
    return true;
}

// Test case for obstacle requirements being minimums
bool testObstacle_MeetsRequirements() {
    Obstacle obstacle(1, 8, 1, "Repeated Failure", ""); // 5 EI + 3 R
    ASSERT_TRUE(!obstacle.meetsRequirements(4, 3));
    ASSERT_TRUE(!obstacle.meetsRequirements(5, 2));
    ASSERT_TRUE(obstacle.meetsRequirements(5, 3));
    ASSERT_TRUE(obstacle.meetsRequirements(6, 4));
    return true;
}

// BDD-style test: EI and R come from the equipment the player carries
bool testPlayerEquipmentDeterminesStats() {
    // GIVEN a room with one EI and one R piece of equipment
    Room room("Armory of the Heart");
    Equipment empathy(1, "Empathy", "", 1, EquipmentKind::EmotionalIntelligence);
    Equipment grit(2, "Grit", "", 1, EquipmentKind::Resilience, 2);
    Tool plain_tool(3, "Wrench", "", 1);
    room.addObject(&empathy);
    room.addObject(&grit);
    room.addObject(&plain_tool);
    Player player(0, "Specialist", "2026-10-14", &room);
    ASSERT_EQ(player.getEmotionalIntelligence(), 0);
    ASSERT_EQ(player.getResilience(), 0);

    // WHEN the player picks up all equipment
    ASSERT_EQ(player.pickUpEquipment(), 2);

    // THEN their stats reflect it, and ordinary tools are left behind
    ASSERT_EQ(player.getEmotionalIntelligence(), 1);
    ASSERT_EQ(player.getResilience(), 2);
    ASSERT_EQ(player.getCarryWeight(), 2);
    ASSERT_EQ(room.getObjects().size(), 1u);

    // WHEN the player drops a piece of equipment where they stand
    player.setPosition({4, 4});
    player.dropTool(&grit);

    // THEN the stat it provided is lost and it lies on the player's tile
    ASSERT_EQ(player.getResilience(), 0);
    ASSERT_EQ(player.getCarryWeight(), 1);
    ASSERT_TRUE(grit.getPosition() == Position(4, 4));
    return true;
}

// Test case: health never goes below 0 or above the maximum, and statuses count down
bool testPlayer_HealthAndStatus() {
    Player player(0, "Specialist", "2026-10-14", nullptr);
    ASSERT_EQ(player.getHealth(), Player::kDefaultMaxHealth);
    ASSERT_EQ(player.damage(30), 30);
    ASSERT_EQ(player.heal(50), 30); // Only up to the maximum
    ASSERT_EQ(player.damage(250), 100);
    ASSERT_TRUE(player.isDefeated());
    player.restoreHealth();
    ASSERT_EQ(player.getHealth(), 100);
    player.setMaxHealth(130);
    ASSERT_EQ(player.getHealth(), 130);
    ASSERT_EQ(player.damage(-5), 0);

    player.addStatus(StatusEffect::Shaken, 2);
    ASSERT_TRUE(player.hasStatus(StatusEffect::Shaken));
    player.addStatus(StatusEffect::Shaken, 1); // Never shortens an effect
    ASSERT_EQ(player.getStatusTurns(StatusEffect::Shaken), 2);
    player.tickStatuses();
    ASSERT_EQ(player.getStatusTurns(StatusEffect::Shaken), 1);
    player.tickStatuses();
    ASSERT_TRUE(!player.hasStatus(StatusEffect::Shaken));
    return true;
}

// BDD-style test: the player cannot carry more than their capacity
bool testPlayerCarryCapacity() {
    // GIVEN a player who can carry 2 and a room with 3 pieces of equipment
    Room room("Storeroom");
    Equipment a(1, "A", "", 1, EquipmentKind::EmotionalIntelligence);
    Equipment b(2, "B", "", 1, EquipmentKind::EmotionalIntelligence);
    Equipment c(3, "C", "", 1, EquipmentKind::Resilience);
    room.addObject(&a);
    room.addObject(&b);
    room.addObject(&c);
    Player player(0, "Specialist", "2026-10-14", &room);
    ASSERT_EQ(player.getCarryCapacity(), Player::kDefaultCarryCapacity);
    player.setCarryCapacity(2);

    // WHEN they pick up everything
    ASSERT_EQ(player.pickUpEquipment(), 2);

    // THEN the last piece stays in the room
    ASSERT_EQ(player.getCarryWeight(), 2);
    ASSERT_EQ(room.getObjects().size(), 1u);
    ASSERT_TRUE(!player.takeTool(&c));
    ASSERT_TRUE(!player.takeTool(nullptr));
    return true;
}

// BDD-style test: an obstacle blocks its exit until the player can clear it, and clearing spends equipment
bool testObstacleBlocksUntilCleared() {
    // GIVEN two rooms with a type-2 obstacle blocking the way north
    Room room1("Start");
    Room room2("Beyond");
    room1.addExit("north", &room2);
    room2.addExit("south", &room1);
    Obstacle obstacle(1, 2, 1, "Impulsive Anger", "", '!', "north");
    Equipment awareness(2, "Awareness", "", 1, EquipmentKind::EmotionalIntelligence);
    Equipment regulation(3, "Regulation", "", 1, EquipmentKind::EmotionalIntelligence);
    Equipment optimism(4, "Optimism", "", 1, EquipmentKind::Resilience);
    room1.addObject(&obstacle);
    room1.addObject(&awareness);
    Player player(0, "Specialist", "2026-10-14", &room1);

    // THEN the player cannot pass or clear it empty-handed
    ASSERT_EQ(player.getBlockingObstacle("north"), &obstacle);
    ASSERT_TRUE(!player.move("north"));
    ASSERT_TRUE(!player.clearObstacle(&obstacle));

    // WHEN the player has only 1 of the 2 EI required
    player.pickUpEquipment();
    // THEN it still blocks them
    ASSERT_TRUE(!player.canClear(&obstacle));
    ASSERT_TRUE(!player.move("north"));

    // WHEN the player has enough EI, plus some R the obstacle does not need
    room1.addObject(&regulation);
    room1.addObject(&optimism);
    player.pickUpEquipment();
    ASSERT_TRUE(player.clearObstacle(&obstacle));

    // THEN the EI is spent and consumed, the unneeded R is kept
    ASSERT_EQ(player.getEmotionalIntelligence(), 0);
    ASSERT_EQ(player.getResilience(), 1);
    ASSERT_TRUE(awareness.isConsumed());
    ASSERT_TRUE(regulation.isConsumed());
    ASSERT_TRUE(!optimism.isConsumed());
    ASSERT_EQ(player.getTools().size(), 1u);

    // AND the obstacle is gone and the player can move on
    ASSERT_TRUE(obstacle.isCleared());
    ASSERT_EQ(room1.getObjects().size(), 0u);
    ASSERT_EQ(player.getObstaclesCleared(), 1);
    ASSERT_EQ(player.getScore(), 2);
    ASSERT_TRUE(player.move("north"));
    ASSERT_EQ(player.getCurrentRoom(), &room2);
    ASSERT_TRUE(!player.clearObstacle(&obstacle)); // Cannot be cleared twice
    return true;
}

// BDD-style test: a placed player must stand next to a placed obstacle to clear it
bool testPlayerMustBeAdjacentToClear() {
    Room room("Courtyard");
    Obstacle obstacle(1, 1, 1, "Self-Doubt", "");
    obstacle.setPosition({6, 0});
    Equipment awareness(2, "Awareness", "", 1, EquipmentKind::EmotionalIntelligence);
    room.addObject(&obstacle);
    room.addObject(&awareness);
    Player player(0, "Specialist", "2026-10-14", &room);
    player.pickUpEquipment();

    player.setPosition({6, 2});
    ASSERT_TRUE(!player.canClear(&obstacle));
    player.setPosition({5, 1}); // Diagonally adjacent
    ASSERT_TRUE(player.canClear(&obstacle));
    ASSERT_TRUE(player.clearObstacle(&obstacle));
    return true;
}

// Test case: statistics unlock achievements once and survive a write/read round trip
bool testPlayerStats_AchievementsAndPersistence() {
    PlayerStats stats;
    ASSERT_TRUE(stats.getAchievements().empty());
    std::vector<Achievement> unlocked = stats.recordClear(1, 1, 0, 4);
    ASSERT_EQ(unlocked.size(), 1u);
    ASSERT_TRUE(unlocked[0] == Achievement::FirstStep);
    ASSERT_TRUE(stats.recordClear(1, 1, 0, 4).size() == 1u); // 2 of 4 cleared: Halfway There
    ASSERT_TRUE(stats.hasAchievement(Achievement::HalfwayThere));
    ASSERT_TRUE(stats.recordClear(5, 5, 0, 4)[0] == Achievement::EmotionallyIntelligent);
    unlocked = stats.recordClear(6, 5, 1, 4);
    ASSERT_EQ(unlocked.size(), 3u); // Bouncing Back, Unbreakable, and Unbroken as nobody collapsed
    ASSERT_EQ(stats.getAchievements().size(), 6u);
    stats.recordPickup(EquipmentKind::Resilience);
    stats.recordRoomReached(3);
    stats.recordTurn();

    std::stringstream buffer;
    stats.write(buffer);
    PlayerStats loaded;
    ASSERT_TRUE(loaded.read(buffer));
    ASSERT_EQ(loaded.getObstaclesCleared(), 4);
    ASSERT_EQ(loaded.getObstaclesCleared(1), 2);
    ASSERT_EQ(loaded.getEmotionalIntelligenceSpent(), 12);
    ASSERT_EQ(loaded.getResilienceSpent(), 1);
    ASSERT_EQ(loaded.getResilienceCollected(), 1);
    ASSERT_EQ(loaded.getFurthestRoom(), 3);
    ASSERT_EQ(loaded.getTurns(), 1);
    ASSERT_EQ(loaded.getAchievements().size(), 6u);

    std::stringstream truncated("1 2 3");
    ASSERT_TRUE(!loaded.read(truncated));
    ASSERT_EQ(loaded.getObstaclesCleared(), 4); // Unchanged by a failed read
    return true;
}

namespace {

int cost(const Obstacle* obstacle) {
    return obstacle->getRequiredEmotionalIntelligence() + obstacle->getRequiredResilience();
}

// Clears every obstacle in the player's room in order, as Player-level code would.
bool clearRoomAsPlayer(Environment& env, Player& player, int number, int& collected) {
    for (Obstacle* obstacle : env.getObstacles(number)) {
        ASSERT_EQ(env.getActiveObstacle(number), obstacle);
        ASSERT_TRUE(!player.move(Environment::kForward)); // Still blocked
        collected += player.pickUpEquipment();
        ASSERT_EQ(player.getEmotionalIntelligence(), obstacle->getRequiredEmotionalIntelligence());
        ASSERT_EQ(player.getResilience(), obstacle->getRequiredResilience());
        ASSERT_TRUE(player.clearObstacle(obstacle));
        ASSERT_EQ(player.getCarryWeight(), 0);
        env.spawnEquipmentForActiveObstacle(number);
    }
    ASSERT_TRUE(env.isRoomCleared(number));
    return true;
}

} // namespace

// Test case: every room has more or as many obstacles as the last and costs exactly one more EI+R
bool testEnvironment_RoomsGrowHarder() {
    Environment env(kSqlDir);
    ASSERT_EQ(env.getRoomCount(), 100);
    ASSERT_EQ(env.getObstacleCount(), 715);
    ASSERT_TRUE(env.getObstacles(0).empty());
    ASSERT_TRUE(env.getObstacles(101).empty());
    ASSERT_EQ(env.getRoom(1)->getExit(Environment::kBack), nullptr);
    ASSERT_EQ(env.getRoom(100)->getExit(Environment::kForward), nullptr);

    size_t previous_count = 0;
    int previous_cost = 0;
    int previous_hardest = 0;
    std::map<int, int> per_type;
    for (int number = 1; number <= 100; ++number) {
        const std::vector<Obstacle*>& obstacles = env.getObstacles(number);
        const RoomLayout& layout = env.getLayout(number);
        int room_cost = 0;
        for (size_t i = 0; i < obstacles.size(); ++i) {
            room_cost += cost(obstacles[i]);
            per_type[obstacles[i]->getType()]++;
            ASSERT_EQ(obstacles[i]->getRoomId(), number);
            // Easiest first, and the last one stands on the door
            ASSERT_TRUE(i == 0 || obstacles[i - 1]->getType() <= obstacles[i]->getType());
            ASSERT_TRUE(obstacles[i]->getPosition() == (i + 1 == obstacles.size() ? layout.door : layout.obstacle_points[i]));
        }
        ASSERT_EQ(room_cost, number); // Room n costs n EI+R: strictly harder than room n - 1
        ASSERT_TRUE(room_cost > previous_cost);
        ASSERT_TRUE(obstacles.size() >= previous_count);
        ASSERT_TRUE(obstacles.back()->getType() >= previous_hardest);
        previous_cost = room_cost;
        previous_count = obstacles.size();
        previous_hardest = obstacles.back()->getType();
        if (number < 100) {
            ASSERT_EQ(env.getRoom(number)->getExit(Environment::kForward), env.getRoom(number + 1));
            ASSERT_EQ(env.getRoom(number + 1)->getExit(Environment::kBack), env.getRoom(number));
        }
    }
    ASSERT_EQ(env.getObstacles(1).size(), 1u);
    ASSERT_EQ(env.getObstacles(50).size(), 8u);
    ASSERT_EQ(env.getObstacles(100).size(), 10u);
    std::map<int, int> expected = {{1, 5}, {2, 14}, {3, 27}, {4, 44}, {5, 65}, {6, 90}, {7, 119}, {8, 152}, {9, 144}, {10, 55}};
    ASSERT_TRUE(per_type == expected); // All ten types appear
    ASSERT_TRUE(!env.isComplete());

    ASSERT_EQ(env.getRoomName(13), "Ember Corridor III");
    ASSERT_EQ(env.getRoomNumber(env.getRoom(42)), 42);
    ASSERT_EQ(env.getRoomNumber(nullptr), 0);
    ASSERT_EQ(env.getExitDescription(1, "north"), "A doorway leads on to Hall of Echoes II.");
    ASSERT_EQ(env.getExitDescription(1, "south"), "");
    ASSERT_EQ(env.getObstacleName(9), "Isolation");
    return true;
}

// Test case: each obstacle type is drawn with its own symbol
bool testEnvironment_ObstacleTypesAreDistinct() {
    Environment env(kSqlDir);
    std::map<int, char> symbol_of_type;
    std::set<char> symbols;
    for (const auto& obstacle : env.getAllObstacles()) {
        auto [entry, added] = symbol_of_type.emplace(obstacle->getType(), obstacle->getSymbol());
        ASSERT_EQ(entry->second, obstacle->getSymbol()); // Every obstacle of a type looks the same
        if (added) {
            symbols.insert(obstacle->getSymbol());
        }
    }
    ASSERT_EQ(symbol_of_type.size(), 10u);
    ASSERT_EQ(symbols.size(), 10u); // No two types share a symbol
    for (char symbol : symbols) {
        ASSERT_TRUE(std::string(WorldData::kReservedSymbols).find(symbol) == std::string::npos);
    }
    ASSERT_EQ(env.getObstacleSymbol(1), '?');
    ASSERT_EQ(env.getObstacleSymbol(10), '*');
    ASSERT_EQ(env.getObstacleSymbol(11), 0);
    return true;
}

// Test case: the spawn points hold exactly the active obstacle's equipment, and refill for the next
bool testEnvironment_FixedSpawnPointsRefillPerObstacle() {
    Environment env(kSqlDir);
    // The shipped spawn points (sql/gauntlet_spawn_points.csv)
    const std::vector<Position> ei_spawns = {{2, 2}, {4, 2}, {6, 2}, {8, 2}, {10, 2}};
    const std::vector<Position> r_spawns = {{2, 4}, {4, 4}, {6, 4}, {8, 4}, {10, 4}};
    int total_ei = 0;
    int total_r = 0;
    for (int number = 1; number <= env.getRoomCount(); ++number) {
        const RoomLayout& layout = env.getLayout(number);
        ASSERT_TRUE(layout.ei_spawns == ei_spawns);
        ASSERT_TRUE(layout.r_spawns == r_spawns);

        for (const Obstacle* obstacle : env.getObstacles(number)) {
            // Each obstacle's wave fills the first N spawn points of each kind, in order
            std::vector<Position> wave_ei;
            std::vector<Position> wave_r;
            for (const Equipment* piece : env.getEquipmentFor(obstacle)) {
                ASSERT_EQ(piece->getRoomId(), number);
                (piece->getKind() == EquipmentKind::EmotionalIntelligence ? wave_ei : wave_r).push_back(piece->getPosition());
                // Only the first obstacle's wave is out when the room is built
                ASSERT_EQ(piece->isSpawned(), obstacle == env.getObstacles(number).front());
            }
            ASSERT_TRUE(wave_ei == std::vector<Position>(ei_spawns.begin(), ei_spawns.begin() + obstacle->getRequiredEmotionalIntelligence()));
            ASSERT_TRUE(wave_r == std::vector<Position>(r_spawns.begin(), r_spawns.begin() + obstacle->getRequiredResilience()));
            total_ei += static_cast<int>(wave_ei.size());
            total_r += static_cast<int>(wave_r.size());
        }
        size_t on_map = 0;
        for (RoomObject* object : env.getRoom(number)->getObjects()) {
            on_map += dynamic_cast<Equipment*>(object) ? 1 : 0;
        }
        ASSERT_EQ(on_map, env.getEquipmentFor(env.getActiveObstacle(number)).size());
    }
    ASSERT_EQ(total_ei, 3415);
    ASSERT_EQ(total_r, 1635);
    ASSERT_EQ(env.getEquipment().size(), 5050u);

    // Clearing room 3's first obstacle brings out the second one's equipment, once
    Player player(0, "Specialist", "2026-10-14", env.getRoom(3));
    player.pickUpEquipment();
    ASSERT_TRUE(player.clearObstacle(env.getObstacles(3)[0]));
    ASSERT_EQ(env.spawnEquipmentForActiveObstacle(3), 2); // Type 2 needs 2 EI
    ASSERT_EQ(env.spawnEquipmentForActiveObstacle(3), 0);
    ASSERT_EQ(player.pickUpEquipment(), 2);

    // Two environments are identical: spawn points never vary between playthroughs
    Environment other(kSqlDir);
    Environment fresh(kSqlDir);
    for (size_t i = 0; i < fresh.getEquipment().size(); ++i) {
        ASSERT_TRUE(fresh.getEquipment()[i]->getPosition() == other.getEquipment()[i]->getPosition());
        ASSERT_EQ(fresh.getEquipment()[i]->getName(), other.getEquipment()[i]->getName());
    }
    return true;
}

// BDD-style test: play through all 100 rooms collecting EI and R and clearing every obstacle
bool testPlayThroughAllHundredRooms() {
    // GIVEN a fresh 100-room environment and an unequipped player at the start
    Environment env(kSqlDir);
    Player player(0, "Integration Specialist", "2026-10-14", env.getStartingRoom());
    int equipment_collected = 0;

    for (int number = 1; number <= env.getRoomCount(); ++number) {
        ASSERT_EQ(player.getCurrentRoom(), env.getRoom(number));
        // WHEN the player works through each obstacle with the equipment spawned for it
        if (!clearRoomAsPlayer(env, player, number, equipment_collected)) {
            std::cerr << "Could not clear room " << number << std::endl;
            return false;
        }
        // THEN the way forward opens
        ASSERT_EQ(player.getBlockingObstacle(Environment::kForward), nullptr);
        if (number < env.getRoomCount()) {
            ASSERT_TRUE(player.move(Environment::kForward));
        }
    }

    // THEN the player finishes in the final room with every obstacle cleared and every piece spent
    ASSERT_EQ(player.getCurrentRoom(), env.getFinalRoom());
    ASSERT_TRUE(env.isComplete());
    ASSERT_EQ(player.getObstaclesCleared(), 715);
    ASSERT_EQ(equipment_collected, 5050);
    for (const auto& equipment : env.getEquipment()) {
        ASSERT_TRUE(equipment->isConsumed());
    }
    ASSERT_EQ(player.getScore(), 5050); // Each obstacle scores its type: the sum of 1..100
    return true;
}

// BDD-style test: a player who leaves the resilience equipment behind is stopped at the first R obstacle
bool testSkippingEquipmentBlocksProgress() {
    // GIVEN a player who has cleared the first 30 rooms
    Environment env(kSqlDir);
    Player player(0, "Specialist", "2026-10-14", env.getStartingRoom());
    int collected = 0;
    for (int number = 1; number <= 30; ++number) {
        ASSERT_TRUE(clearRoomAsPlayer(env, player, number, collected));
        ASSERT_TRUE(player.move(Environment::kForward));
    }

    // AND has worked through room 31 up to its last obstacle, the first that needs resilience
    const std::vector<Obstacle*>& obstacles = env.getObstacles(31);
    for (size_t i = 0; i + 1 < obstacles.size(); ++i) {
        ASSERT_EQ(obstacles[i]->getRequiredResilience(), 0);
        player.pickUpEquipment();
        ASSERT_TRUE(player.clearObstacle(obstacles[i]));
        env.spawnEquipmentForActiveObstacle(31);
    }
    Obstacle* setback = env.getActiveObstacle(31);
    ASSERT_EQ(setback->getType(), 6);

    // WHEN they collect only the EI equipment for it
    std::vector<RoomObject*> objects = player.getCurrentRoom()->getObjects();
    for (RoomObject* object : objects) {
        Equipment* equipment = dynamic_cast<Equipment*>(object);
        if (equipment && equipment->getKind() == EquipmentKind::EmotionalIntelligence) {
            ASSERT_TRUE(player.takeTool(equipment));
        }
    }
    ASSERT_EQ(player.getEmotionalIntelligence(), 5);

    // THEN they cannot clear it or move past it
    ASSERT_TRUE(!player.clearObstacle(setback));
    ASSERT_TRUE(!player.move(Environment::kForward));

    // AND retreating is still possible
    ASSERT_TRUE(player.move(Environment::kBack));
    ASSERT_EQ(player.getCurrentRoom(), env.getRoom(30));
    return true;
}

// Function to register all environment tests with the runner
void registerEnvironmentTests(TestRunner& runner) {
    runner.addTest("testPosition_StepAndDistance", testPosition_StepAndDistance);
    runner.addTest("testRoomObject_Position", testRoomObject_Position);
    runner.addTest("testEquipment_Creation", testEquipment_Creation);
    runner.addTest("testObstacle_RequirementsForAllTypes", testObstacle_RequirementsForAllTypes);
    runner.addTest("testObstacle_RejectsInvalidType", testObstacle_RejectsInvalidType);
    runner.addTest("testObstacle_MeetsRequirements", testObstacle_MeetsRequirements);
    runner.addTest("testPlayerEquipmentDeterminesStats", testPlayerEquipmentDeterminesStats);
    runner.addTest("testPlayer_HealthAndStatus", testPlayer_HealthAndStatus);
    runner.addTest("testPlayerCarryCapacity", testPlayerCarryCapacity);
    runner.addTest("testObstacleBlocksUntilCleared", testObstacleBlocksUntilCleared);
    runner.addTest("testPlayerMustBeAdjacentToClear", testPlayerMustBeAdjacentToClear);
    runner.addTest("testPlayerStats_AchievementsAndPersistence", testPlayerStats_AchievementsAndPersistence);
    runner.addTest("testEnvironment_RoomsGrowHarder", testEnvironment_RoomsGrowHarder);
    runner.addTest("testEnvironment_ObstacleTypesAreDistinct", testEnvironment_ObstacleTypesAreDistinct);
    runner.addTest("testEnvironment_FixedSpawnPointsRefillPerObstacle", testEnvironment_FixedSpawnPointsRefillPerObstacle);
    runner.addTest("testPlayThroughAllHundredRooms", testPlayThroughAllHundredRooms);
    runner.addTest("testSkippingEquipmentBlocksProgress", testSkippingEquipmentBlocksProgress);
}
