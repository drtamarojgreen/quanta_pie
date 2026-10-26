#include "TestRunner.h"
#include "TestData.h"
#include "../src/environments/Gauntlet.h"
#include "../src/environments/Hud.h"
#include <cstdio>
#include <filesystem>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace {

int sign(int value) {
    return (value > 0) - (value < 0);
}

// Presses the first key bound to a command, exactly as a player would.
bool press(Gauntlet& game, Command command) {
    std::vector<char> keys = game.getKeyBindings().keysFor(command);
    return !keys.empty() && game.handleKey(keys.front());
}

Command moveToward(int dx, int dy) {
    if (dy < 0) return dx < 0 ? Command::MoveNorthWest : dx > 0 ? Command::MoveNorthEast : Command::MoveNorth;
    if (dy > 0) return dx < 0 ? Command::MoveSouthWest : dx > 0 ? Command::MoveSouthEast : Command::MoveSouth;
    return dx < 0 ? Command::MoveWest : Command::MoveEast;
}

// The first step of the cheapest route to a tile, as a careful player would walk it: each step
// costs 1 and a hazard its damage on top; uncleared obstacles cannot be crossed.
Position nextStepToward(const Gauntlet& game, const Position& target) {
    const RoomLayout& layout = game.getCurrentLayout();
    auto index = [&layout](const Position& p) { return p.y * layout.width + p.x; };
    auto passable = [&game](const Position& p) {
        for (RoomObject* object : game.getCurrentRoom()->getObjects()) {
            const Obstacle* obstacle = dynamic_cast<const Obstacle*>(object);
            if (obstacle && !obstacle->isCleared() && obstacle->getPosition() == p) {
                return false;
            }
        }
        return true;
    };
    const Direction directions[] = {Direction::North, Direction::NorthEast, Direction::East, Direction::SouthEast,
                                    Direction::South, Direction::SouthWest, Direction::West, Direction::NorthWest};
    std::vector<int> cost(layout.width * layout.height, 1 << 30);
    std::vector<Position> previous(cost.size());
    std::set<std::pair<int, int>> frontier; // (cost, tile index), cheapest first
    Position start = game.getPlayer().getPosition();
    cost[index(start)] = 0;
    frontier.insert({0, index(start)});
    while (!frontier.empty()) {
        auto [here_cost, here_index] = *frontier.begin();
        frontier.erase(frontier.begin());
        Position here(here_index % layout.width, here_index / layout.width);
        for (Direction direction : directions) {
            Position next = here.step(direction);
            if (!layout.contains(next) || !passable(next)) {
                continue;
            }
            const Hazard* hazard = layout.hazardAt(next);
            int next_cost = here_cost + 1 + (hazard ? hazard->damage : 0);
            if (next_cost < cost[index(next)]) {
                frontier.erase({cost[index(next)], index(next)});
                cost[index(next)] = next_cost;
                previous[index(next)] = here;
                frontier.insert({next_cost, index(next)});
            }
        }
    }
    if (!layout.contains(target) || cost[index(target)] == (1 << 30)) {
        return Position();
    }
    Position step = target;
    while (previous[index(step)] != start) {
        step = previous[index(step)];
    }
    return step;
}

// Walks the cheapest route to a tile in the current room, one key press per step.
bool walkTo(Gauntlet& game, const Position& target) {
    for (int guard = 0; guard < 64 && game.getPlayer().getPosition() != target; ++guard) {
        Position at = game.getPlayer().getPosition();
        Position next = nextStepToward(game, target);
        if (!next.isValid() || !press(game, moveToward(sign(next.x - at.x), sign(next.y - at.y)))) {
            return false;
        }
    }
    return game.getPlayer().getPosition() == target;
}

Position besideDoor(const RoomLayout& layout) {
    return {layout.door.x, layout.door.y + 1};
}

// The tile just south of an obstacle on the north wall.
Position beside(const Obstacle* obstacle) {
    return {obstacle->getPosition().x, obstacle->getPosition().y + 1};
}

// Collects the spawns for the room's next obstacle and clears it.
bool clearActiveObstacle(Gauntlet& game) {
    const RoomLayout& layout = game.getCurrentLayout();
    Obstacle* obstacle = game.getActiveObstacle();
    ASSERT_TRUE(obstacle != nullptr);
    int need_ei = obstacle->getRequiredEmotionalIntelligence();
    int need_r = obstacle->getRequiredResilience();

    for (int i = 0; i < need_ei; ++i) {
        ASSERT_TRUE(walkTo(game, layout.ei_spawns[i]));
    }
    for (int i = 0; i < need_r; ++i) {
        ASSERT_TRUE(walkTo(game, layout.r_spawns[i]));
    }
    ASSERT_EQ(game.getPlayer().getEmotionalIntelligence(), need_ei);
    ASSERT_EQ(game.getPlayer().getResilience(), need_r);

    ASSERT_TRUE(walkTo(game, beside(obstacle)));
    ASSERT_TRUE(press(game, Command::Clear));
    ASSERT_TRUE(obstacle->isCleared());
    ASSERT_EQ(game.getPlayer().getEmotionalIntelligence(), 0);
    ASSERT_EQ(game.getPlayer().getResilience(), 0);
    return true;
}

// Clears every obstacle in the current room in order and walks through the north door.
bool clearCurrentRoom(Gauntlet& game) {
    int number = game.getCurrentRoomNumber();
    while (game.getActiveObstacle()) {
        ASSERT_TRUE(clearActiveObstacle(game));
    }
    Room* next = game.getCurrentRoom()->getExit(Environment::kForward);
    if (next) {
        ASSERT_TRUE(walkTo(game, game.getCurrentLayout().door));
        ASSERT_TRUE(press(game, Command::MoveNorth));
        ASSERT_EQ(game.getCurrentRoom(), next);
        ASSERT_TRUE(game.getEnvironment().isRoomCleared(number));
    }
    return true;
}

bool drainEventsContain(Gauntlet& game, GameEventType type) {
    bool found = false;
    GameEvent event;
    while (game.pollEvent(event)) {
        found = found || event.type == type;
    }
    return found;
}

// A fresh, empty directory under the system temp directory for save slots.
std::string tempDirectory(const std::string& name) {
    std::filesystem::path directory = std::filesystem::temp_directory_path() / ("quantapie_saves_" + name);
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);
    return directory.string();
}

GameOptions savingTo(const std::string& directory) {
    GameOptions options;
    options.save_directory = directory;
    return options;
}

// Replaces the value of one "label value" line in a save file's text.
std::string withLine(std::string text, const std::string& label, const std::string& value) {
    size_t start = text.find("\n" + label + " ") + 1;
    size_t end = text.find('\n', start);
    return text.replace(start, end - start, label + " " + value);
}

bool logContains(const Gauntlet& game, const std::string& message) {
    for (const std::string& line : game.getMessageLog().getMessages()) {
        if (line == message) {
            return true;
        }
    }
    std::cerr << "Not in the message log: " << message << std::endl;
    return false;
}

// Walks back and forth beside the entry until the player collapses (or the run ends).
bool walkUntilCollapse(Gauntlet& game) {
    ASSERT_TRUE(walkTo(game, game.getCurrentLayout().entry));
    int collapses = game.getStats().getCollapses();
    for (int guard = 0; guard < 1000; ++guard) {
        if (game.getStats().getCollapses() > collapses || game.getState() == GameState::Defeated) {
            return true;
        }
        Position entry = game.getCurrentLayout().entry;
        Command move = game.getPlayer().getPosition() == entry ? Command::MoveWest : Command::MoveEast;
        if (!press(game, move)) {
            return false;
        }
    }
    return false;
}

} // namespace

// Test case: default key bindings, rebinding and loading a bindings config
bool testKeyBindings_DefaultsRebindAndLoad() {
    KeyBindings bindings;
    ASSERT_TRUE(bindings.commandFor('w') == Command::MoveNorth);
    ASSERT_TRUE(bindings.commandFor('q') == Command::MoveNorthWest);
    ASSERT_TRUE(bindings.commandFor('s') == Command::Wait);
    ASSERT_TRUE(bindings.commandFor('f') == Command::Clear);
    ASSERT_TRUE(bindings.commandFor('?') == Command::None);
    ASSERT_TRUE(bindings.commandFor(' ') == Command::None);

    bindings.bind('k', Command::MoveNorth);
    bindings.unbind('w');
    ASSERT_TRUE(bindings.commandFor('w') == Command::None);
    ASSERT_TRUE(bindings.keysFor(Command::MoveNorth) == std::vector<char>{'k'});

    std::istringstream config("# arrow-free layout\n8 = move_north\n2 = move_south\n\nf = none\n");
    ASSERT_TRUE(bindings.load(config));
    ASSERT_TRUE(bindings.commandFor('8') == Command::MoveNorth);
    ASSERT_TRUE(bindings.commandFor('2') == Command::MoveSouth);
    ASSERT_TRUE(bindings.commandFor('f') == Command::None);

    std::istringstream bad("9 = move_north\nzz = wait\n");
    ASSERT_TRUE(!bindings.load(bad));
    ASSERT_TRUE(bindings.commandFor('9') == Command::None); // A bad config changes nothing

    bindings.resetToDefaults();
    ASSERT_TRUE(bindings.commandFor('w') == Command::MoveNorth);
    ASSERT_TRUE(KeyBindings::commandFromName(KeyBindings::commandName(Command::Save)) == Command::Save);
    return true;
}

// BDD-style test: a new game starts at the entry of room 1 with a clear objective
bool testGauntlet_StartsInFirstRoom() {
    Gauntlet game(kSqlDir);
    ASSERT_EQ(game.getRoomCount(), 100);
    ASSERT_EQ(game.getCurrentRoomNumber(), 1);
    ASSERT_TRUE(game.getState() == GameState::Playing);
    ASSERT_TRUE(game.getPlayer().getPosition() == game.getCurrentLayout().entry);
    ASSERT_EQ(game.getObjective(),
              "Collect 1 EI from the E and R tiles, then stand next to ? Self-Doubt and press 'f'.");
    GameEvent event;
    ASSERT_TRUE(game.pollEvent(event));
    ASSERT_TRUE(event.type == GameEventType::RoomEntered);
    ASSERT_EQ(event.message, "You enter Room 1: Hall of Echoes I. ? Self-Doubt blocks the north door.");
    ASSERT_TRUE(!game.pollEvent(event));
    return true;
}

// BDD-style test: the player moves in eight directions and walls stop them
bool testGauntlet_DiagonalMovementAndWalls() {
    Gauntlet game(kSqlDir);
    // WHEN the player steps northwest from the entry (6,6)
    ASSERT_TRUE(press(game, Command::MoveNorthWest));
    // THEN they move diagonally and a turn passes
    ASSERT_TRUE(game.getPlayer().getPosition() == Position(5, 5));
    ASSERT_EQ(game.getStats().getTurns(), 1);
    ASSERT_TRUE(press(game, Command::MoveSouthEast));
    ASSERT_TRUE(game.getPlayer().getPosition() == Position(6, 6));

    // WHEN they walk into the west wall
    ASSERT_TRUE(walkTo(game, {0, 6}));
    drainEventsContain(game, GameEventType::PickedUp);
    ASSERT_TRUE(!press(game, Command::MoveWest));
    ASSERT_TRUE(!press(game, Command::MoveSouthWest));
    // THEN they stay put, no turn passes, and the blocked moves are recorded
    ASSERT_TRUE(game.getPlayer().getPosition() == Position(0, 6));
    ASSERT_EQ(game.getStats().getBlockedMoves(), 2);
    ASSERT_TRUE(drainEventsContain(game, GameEventType::Blocked));

    // AND there is no way south out of the first room
    ASSERT_TRUE(walkTo(game, game.getCurrentLayout().entry));
    ASSERT_TRUE(!press(game, Command::MoveSouth));
    ASSERT_EQ(game.getCurrentRoomNumber(), 1);
    return true;
}

// BDD-style test: walking onto a fixed spawn point collects the equipment there
bool testGauntlet_CollectsEquipmentFromSpawnPoints() {
    Gauntlet game(kSqlDir);
    const RoomLayout& layout = game.getCurrentLayout();
    ASSERT_TRUE(walkTo(game, layout.ei_spawns[1])); // Nothing spawns on the 2nd EI tile in room 1
    ASSERT_EQ(game.getPlayer().getEmotionalIntelligence(), 0);
    ASSERT_TRUE(walkTo(game, layout.ei_spawns[0]));
    ASSERT_EQ(game.getPlayer().getEmotionalIntelligence(), 1);
    ASSERT_EQ(game.getStats().getEmotionalIntelligenceCollected(), 1);
    ASSERT_EQ(game.getMessageLog().getMessages().back(),
              "You pick up the Mirror of Self-Awareness (+1 EI).");
    ASSERT_EQ(game.getObjective(), "Clear ? Self-Doubt: stand next to ? Self-Doubt and press 'f'.");
    return true;
}

// BDD-style test: the obstacle blocks the door and can only be cleared up close with enough EI and R
bool testGauntlet_ObstacleRules() {
    Gauntlet game(kSqlDir);
    const RoomLayout& layout = game.getCurrentLayout();
    ASSERT_TRUE(walkTo(game, besideDoor(layout)));

    // Walking into the obstacle is blocked
    ASSERT_TRUE(!press(game, Command::MoveNorth));
    ASSERT_TRUE(game.getPlayer().getPosition() == besideDoor(layout));

    // Clearing without the EI it needs fails, and the obstacle strikes back
    int health = game.getPlayer().getHealth();
    ASSERT_TRUE(!press(game, Command::Clear));
    ASSERT_TRUE(logContains(game, "Self-Doubt needs 1 EI; you carry 0 EI and 0 R."));
    ASSERT_TRUE(logContains(game, "Self-Doubt strikes back: -2 health."));
    ASSERT_EQ(game.getPlayer().getHealth(), health - 2);
    ASSERT_TRUE(game.getPlayer().hasStatus(StatusEffect::Shaken));
    ASSERT_EQ(game.getStats().getStrikesTaken(), 1);

    // Clearing from a distance fails even when equipped
    ASSERT_TRUE(walkTo(game, layout.ei_spawns[0]));
    ASSERT_TRUE(!press(game, Command::Clear));
    ASSERT_EQ(game.getMessageLog().getMessages().back(), "Stand next to ? Self-Doubt to work through it.");

    // Up close and equipped, it clears and the equipment is spent
    ASSERT_TRUE(walkTo(game, {5, 1}));
    ASSERT_TRUE(press(game, Command::Clear));
    ASSERT_TRUE(game.getActiveObstacle() == nullptr);
    ASSERT_TRUE(game.getEnvironment().isRoomCleared(1));
    ASSERT_EQ(game.getPlayer().getEmotionalIntelligence(), 0);
    ASSERT_TRUE(game.getStats().hasAchievement(Achievement::FirstStep));
    ASSERT_TRUE(drainEventsContain(game, GameEventType::AchievementUnlocked));
    ASSERT_EQ(game.getObjective(), "Head north through the door to Room 2.");
    ASSERT_EQ(game.getLastHealthChange(), 13); // Type 1 restores 10 + 3
    ASSERT_TRUE(!press(game, Command::Clear)); // Nothing left to clear
    return true;
}

// BDD-style test: a room's obstacles are cleared one at a time, each bringing out the next one's equipment
bool testGauntlet_ObstaclesClearInOrderAndRefillSpawns() {
    Gauntlet game(kSqlDir);
    ASSERT_TRUE(clearCurrentRoom(game));
    ASSERT_TRUE(clearCurrentRoom(game));

    // GIVEN room 3: ? Self-Doubt at (5,0), then ! Impulsive Anger on the door
    ASSERT_EQ(game.getCurrentRoomNumber(), 3);
    const std::vector<Obstacle*>& obstacles = game.getEnvironment().getObstacles(3);
    ASSERT_EQ(obstacles.size(), 2u);
    ASSERT_EQ(obstacles[0]->getLabel(), "? Self-Doubt");
    ASSERT_EQ(obstacles[1]->getLabel(), "! Impulsive Anger");
    ASSERT_EQ(game.getMessageLog().getMessages().back(), "You enter Room 3: Hall of Echoes III. 2 obstacles block the north door.");

    // WHEN the player tries the door obstacle first
    ASSERT_TRUE(walkTo(game, game.getCurrentLayout().ei_spawns[0]));
    ASSERT_TRUE(walkTo(game, {7, 1}));
    ASSERT_TRUE(!press(game, Command::Clear));
    // THEN it has to wait for the first
    ASSERT_EQ(game.getMessageLog().getMessages().back(), "Impulsive Anger has to wait: work through ? Self-Doubt first.");
    ASSERT_TRUE(press(game, Command::Look));
    ASSERT_TRUE(press(game, Command::MoveNorthWest));
    ASSERT_EQ(game.getMessageLog().getMessages().back(),
              "To the northwest: ! Impulsive Anger (type 2, needs 2 EI). A wall of heat flares whenever you "
              "approach the exit. It waits behind ? Self-Doubt.");

    // WHEN the first is cleared
    ASSERT_TRUE(walkTo(game, {5, 1}));
    ASSERT_TRUE(press(game, Command::Clear));
    // THEN the spawn points refill with what the second needs, and the door stays shut
    ASSERT_TRUE(drainEventsContain(game, GameEventType::EquipmentSpawned));
    ASSERT_EQ(game.getActiveObstacle(), obstacles[1]);
    ASSERT_EQ(Hud::renderRoom(game)[1], "|......!......|");
    ASSERT_EQ(Hud::renderRoom(game)[3], "|..E.E........|");
    ASSERT_TRUE(!press(game, Command::MoveNorthEast)); // Onto the door obstacle
    ASSERT_EQ(game.getObjective(), "Collect 2 EI from the E and R tiles, then stand next to ! Impulsive Anger and press 'f'.");

    // WHEN the second is cleared too, THEN the door opens
    ASSERT_TRUE(clearActiveObstacle(game));
    // Clearing a type 2 restores up to 16 health, capped at the maximum
    std::string cleared = game.getMessageLog().getMessages().back();
    ASSERT_EQ(cleared.substr(0, 49), "You work through Impulsive Anger, spending 2 EI (");
    ASSERT_EQ(cleared.substr(cleared.size() - 31), "health). The way north is open.");
    ASSERT_TRUE(game.getLastHealthChange() > 0 && game.getLastHealthChange() <= 16);
    ASSERT_TRUE(walkTo(game, game.getCurrentLayout().door));
    ASSERT_TRUE(press(game, Command::MoveNorth));
    ASSERT_EQ(game.getCurrentRoomNumber(), 4);
    return true;
}

// BDD-style test: looking reveals a neighbouring tile without moving or taking a turn
bool testGauntlet_LookWithoutMoving() {
    Gauntlet game(kSqlDir);
    ASSERT_TRUE(walkTo(game, besideDoor(game.getCurrentLayout())));
    Position before = game.getPlayer().getPosition();
    int turns = game.getStats().getTurns();

    ASSERT_TRUE(press(game, Command::Look));
    ASSERT_TRUE(game.getState() == GameState::Looking);
    ASSERT_TRUE(press(game, Command::MoveNorth));
    ASSERT_TRUE(game.getState() == GameState::Playing);
    ASSERT_EQ(game.getMessageLog().getMessages().back(),
              "To the north: ? Self-Doubt (type 1, needs 1 EI). A fog of 'I can't do this' hangs across the doorway.");
    ASSERT_TRUE(game.getPlayer().getPosition() == before);
    ASSERT_EQ(game.getStats().getTurns(), turns);

    ASSERT_TRUE(press(game, Command::Look));
    ASSERT_TRUE(press(game, Command::MoveSouth));
    ASSERT_EQ(game.getMessageLog().getMessages().back(), "To the south: open floor.");

    // Any non-direction key cancels looking
    ASSERT_TRUE(press(game, Command::Look));
    ASSERT_TRUE(!press(game, Command::Wait));
    ASSERT_TRUE(game.getState() == GameState::Playing);
    ASSERT_EQ(game.getStats().getWaits(), 0);
    return true;
}

// BDD-style test: waiting passes a turn; pausing freezes play until resumed
bool testGauntlet_WaitAndPause() {
    Gauntlet game(kSqlDir);
    ASSERT_TRUE(press(game, Command::Wait));
    ASSERT_EQ(game.getStats().getTurns(), 1);
    ASSERT_EQ(game.getStats().getWaits(), 1);

    ASSERT_TRUE(press(game, Command::Pause));
    ASSERT_TRUE(game.getState() == GameState::Paused);
    ASSERT_TRUE(!press(game, Command::MoveNorth));
    ASSERT_TRUE(!press(game, Command::Wait));
    ASSERT_TRUE(game.getPlayer().getPosition() == game.getCurrentLayout().entry);
    ASSERT_EQ(game.getStats().getTurns(), 1);

    ASSERT_TRUE(press(game, Command::Pause));
    ASSERT_TRUE(game.getState() == GameState::Playing);
    ASSERT_TRUE(press(game, Command::MoveNorth));
    return true;
}

// BDD-style test: going back keeps the world as the player left it
bool testGauntlet_BacktrackKeepsWorldState() {
    Gauntlet game(kSqlDir);
    ASSERT_TRUE(clearCurrentRoom(game));
    ASSERT_EQ(game.getCurrentRoomNumber(), 2);
    ASSERT_TRUE(game.getPlayer().getPosition() == game.getCurrentLayout().entry);

    // WHEN the player steps back south from the entry
    ASSERT_TRUE(press(game, Command::MoveSouth));

    // THEN they are at room 1's door, its obstacle stays cleared and nothing respawns
    ASSERT_EQ(game.getCurrentRoomNumber(), 1);
    ASSERT_TRUE(game.getPlayer().getPosition() == game.getCurrentLayout().door);
    ASSERT_TRUE(game.getEnvironment().isRoomCleared(1));
    ASSERT_EQ(game.getCurrentRoom()->getObjects().size(), 0u);
    ASSERT_EQ(game.getStats().getBacktracks(), 1);
    ASSERT_EQ(game.getStats().getFurthestRoom(), 2);

    // AND they can go forward again
    ASSERT_TRUE(press(game, Command::MoveNorth));
    ASSERT_EQ(game.getCurrentRoomNumber(), 2);
    return true;
}

// BDD-style test: play through all 100 rooms using only key presses
bool testGauntlet_PlayThroughAllHundredRoomsWithKeys() {
    // GIVEN a new game
    Gauntlet game(kSqlDir);

    // WHEN the player collects each room's spawns, clears its obstacle and moves on
    for (int number = 1; number <= game.getRoomCount(); ++number) {
        ASSERT_EQ(game.getCurrentRoomNumber(), number);
        if (!clearCurrentRoom(game)) {
            std::cerr << "Could not clear room " << number << std::endl;
            return false;
        }
    }

    // THEN every obstacle is cleared and the game is complete
    ASSERT_TRUE(game.getState() == GameState::Completed);
    ASSERT_TRUE(game.getEnvironment().isComplete());
    ASSERT_TRUE(drainEventsContain(game, GameEventType::Completed));
    const PlayerStats& stats = game.getStats();
    ASSERT_EQ(stats.getObstaclesCleared(), 715);
    const int per_type[] = {5, 14, 27, 44, 65, 90, 119, 152, 144, 55};
    for (int type = 1; type <= 10; ++type) {
        ASSERT_EQ(stats.getObstaclesCleared(type), per_type[type - 1]);
    }
    ASSERT_EQ(stats.getEmotionalIntelligenceCollected(), 3415);
    ASSERT_EQ(stats.getEmotionalIntelligenceSpent(), 3415);
    ASSERT_EQ(stats.getResilienceCollected(), 1635);
    ASSERT_EQ(stats.getResilienceSpent(), 1635);
    ASSERT_EQ(stats.getFurthestRoom(), 100);
    ASSERT_EQ(stats.getBlockedMoves(), 0);
    ASSERT_EQ(stats.getAchievements().size(), static_cast<size_t>(Achievement::Count));
    // A careful route never runs out of health, which earns the Unbroken ending
    ASSERT_EQ(stats.getCollapses(), 0);
    ASSERT_TRUE(game.getPlayer().getHealth() > 0);
    ASSERT_TRUE(stats.getDamageTaken() > 0);
    ASSERT_EQ(stats.getHazardsHit(), 0); // The walker steps around every hazard
    ASSERT_TRUE(game.getEnding() == Ending::Unbroken);
    ASSERT_EQ(game.getPlayer().getScore(), 5050);
    ASSERT_EQ(game.getObjective(), "Every obstacle is cleared. You have finished all 100 rooms.");

    // AND further actions are refused, and there is no door past the last room
    ASSERT_TRUE(!press(game, Command::MoveNorth));
    std::vector<std::string> map = Hud::renderMiniMap(game);
    ASSERT_EQ(map.back(), "91  #########@");
    return true;
}

// BDD-style test: saving and loading restores the exact game state
bool testGauntlet_SaveAndLoadRoundTrip() {
    std::string directory = tempDirectory("round_trip");
    Gauntlet game(kSqlDir, savingTo(directory));

    // GIVEN a game in room 56 with two of its obstacles cleared and some EI collected for the third
    for (int number = 1; number <= 55; ++number) {
        ASSERT_TRUE(clearCurrentRoom(game));
    }
    ASSERT_TRUE(clearActiveObstacle(game));
    ASSERT_TRUE(clearActiveObstacle(game));
    ASSERT_TRUE(walkTo(game, game.getCurrentLayout().ei_spawns[0]));
    int saved_cleared = game.getStats().getObstaclesCleared();
    int saved_ei = game.getPlayer().getEmotionalIntelligence(); // The walk there crosses other EI tiles
    ASSERT_TRUE(saved_ei >= 1);
    Position saved_position = game.getPlayer().getPosition();
    int saved_turns = game.getStats().getTurns();
    int saved_score = game.getPlayer().getScore();
    int saved_health = game.getPlayer().getHealth();
    long long saved_time = game.getStats().getElapsedMilliseconds();
    size_t saved_objects = game.getCurrentRoom()->getObjects().size();

    // WHEN the player saves to slot 2 with the save key, plays on, then loads slot 2
    ASSERT_TRUE(press(game, Command::Save));
    ASSERT_TRUE(game.getState() == GameState::ChoosingSaveSlot);
    ASSERT_TRUE(game.handleKey('2'));
    ASSERT_TRUE(std::filesystem::exists(game.slotPath(2)));
    ASSERT_TRUE(clearCurrentRoom(game));
    ASSERT_TRUE(clearCurrentRoom(game));
    ASSERT_EQ(game.getCurrentRoomNumber(), 58);
    ASSERT_TRUE(press(game, Command::Load));
    ASSERT_TRUE(game.handleKey('2'));

    // THEN everything is as it was when saved
    ASSERT_EQ(game.getCurrentRoomNumber(), 56);
    ASSERT_TRUE(game.getPlayer().getPosition() == saved_position);
    ASSERT_EQ(game.getPlayer().getEmotionalIntelligence(), saved_ei);
    ASSERT_EQ(game.getPlayer().getResilience(), 0);
    ASSERT_EQ(game.getPlayer().getScore(), saved_score);
    ASSERT_EQ(game.getPlayer().getHealth(), saved_health);
    ASSERT_TRUE(game.getStats().getElapsedMilliseconds() >= saved_time);
    ASSERT_EQ(game.getCurrentSlot(), 2);
    ASSERT_EQ(game.getStats().getTurns(), saved_turns);
    ASSERT_EQ(game.getStats().getObstaclesCleared(), saved_cleared);
    ASSERT_EQ(game.getCurrentRoom()->getObjects().size(), saved_objects);
    ASSERT_EQ(game.getActiveObstacle(), game.getEnvironment().getObstacles(56)[2]);
    ASSERT_TRUE(game.getEnvironment().isRoomCleared(55));
    ASSERT_EQ(game.getEnvironment().getRoom(55)->getObjects().size(), 0u); // Spent equipment stays spent
    int consumed = 0;
    for (const auto& equipment : game.getEnvironment().getEquipment()) {
        consumed += equipment->isConsumed() ? 1 : 0;
    }
    ASSERT_EQ(consumed, game.getStats().getEmotionalIntelligenceSpent() + game.getStats().getResilienceSpent());
    ASSERT_EQ(game.getEnvironment().getActiveObstacle(57), game.getEnvironment().getObstacles(57)[0]);
    ASSERT_EQ(game.getEnvironment().getRoom(57)->getObjects().size(), 8u + 5u + 2u); // 8 obstacles, first is type 7
    ASSERT_TRUE(drainEventsContain(game, GameEventType::Loaded));

    // AND the loaded game can be finished
    while (game.getState() != GameState::Completed) {
        ASSERT_TRUE(clearCurrentRoom(game));
    }
    ASSERT_EQ(game.getStats().getObstaclesCleared(), 715);
    std::filesystem::remove_all(directory);
    return true;
}

// BDD-style test: corrupt or missing save data is rejected without touching the game
bool testGauntlet_LoadRejectsBadData() {
    std::string directory = tempDirectory("bad_data");
    Gauntlet game(kSqlDir, savingTo(directory));
    ASSERT_TRUE(clearCurrentRoom(game));

    std::stringstream good;
    ASSERT_TRUE(game.save(good));
    std::string text = good.str();

    std::vector<std::string> corrupt = {
        "",
        "NOT_A_SAVE 1\n",
        text.substr(0, text.size() / 2),                                      // Truncated
        std::string(text).replace(text.find("room 2"), 6, "room 0"),          // Room out of range
        std::string(text).replace(text.find("position 6 6"), 12, "position 6 9"), // Off the grid
        std::string(text).replace(text.find("obstacles 1"), 11, "obstacles 7"),   // Bad state digit
        std::string(text).replace(text.find("rooms 100"), 9, "rooms 99"),         // Saved from another world
        std::string(text).replace(text.find("equipment 3"), 11, "equipment 0"),   // First wave marked as waiting
        std::string(text).replace(text.find("QUANTAPIE_GAUNTLET 3"), 20, "QUANTAPIE_GAUNTLET 2"), // An older format
        withLine(text, "health", "0"),                                          // No health left to play with
        withLine(text, "health", "101"),                                        // More than the background allows
        withLine(text, "background", "wizard"),                                 // Not a background
        withLine(text, "hardcore", "2"),
    };
    for (const std::string& data : corrupt) {
        std::istringstream in(data);
        ASSERT_TRUE(!game.load(in));
        ASSERT_EQ(game.getCurrentRoomNumber(), 2);
        ASSERT_TRUE(game.getEnvironment().isRoomCleared(1));
    }
    ASSERT_TRUE(press(game, Command::Load));
    ASSERT_TRUE(!game.handleKey('1')); // No save in slot 1
    ASSERT_TRUE(drainEventsContain(game, GameEventType::LoadFailed));
    std::filesystem::remove_all(directory);

    std::istringstream valid(text);
    ASSERT_TRUE(game.load(valid));
    ASSERT_EQ(game.getCurrentRoomNumber(), 2);
    return true;
}

// Test case: events queue in order for frontends while the message log keeps only the latest
bool testGauntlet_EventQueueAndMessageLog() {
    Gauntlet game(kSqlDir);
    GameEvent event;
    ASSERT_TRUE(game.pollEvent(event)); // Room entered
    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(press(game, Command::Wait));
    }
    int waited = 0;
    while (game.pollEvent(event)) {
        ASSERT_TRUE(event.type == GameEventType::Waited);
        waited++;
    }
    ASSERT_EQ(waited, 10);
    ASSERT_EQ(game.getMessageLog().getMessages().size(), game.getMessageLog().getCapacity());

    MessageLog log(2);
    log.add("one");
    log.add("two");
    log.add("three");
    ASSERT_EQ(log.getMessages().size(), 2u);
    ASSERT_EQ(log.getMessages().front(), "two");
    return true;
}

// Test case: the room renders with the obstacle, the fixed spawn points and the player
bool testHud_RenderRoom() {
    Gauntlet game(kSqlDir);
    for (int number = 1; number <= 60; ++number) {
        ASSERT_TRUE(clearCurrentRoom(game));
    }
    // Room 61: three Sudden Changes (%) then five Repeated Failures (=), the last on the door.
    // The first, a type 7, has its 5 EI and 2 R out.
    std::vector<std::string> expected = {
        "+------^------+",
        "|..==%%=%==...|",
        "|.............|",
        "|..E.E.E.E.E..|",
        "|...:.....:...|",
        "|..R.R........|",
        "|......:......|",
        "|......@......|",
        "+------v------+",
    };
    ASSERT_TRUE(Hud::renderRoom(game) == expected);
    ASSERT_TRUE(walkTo(game, {4, 4}));
    ASSERT_EQ(Hud::renderRoom(game)[5], "|..R.@........|");

    // Once the first obstacle is cleared, its tile is free and the next one's wave is out
    ASSERT_TRUE(clearActiveObstacle(game));
    ASSERT_EQ(Hud::renderRoom(game)[1], "|..==%.=%==...|");
    ASSERT_EQ(Hud::renderRoom(game)[3], "|..E.E.E.E.E..|");
    return true;
}

// Test case: the status panel, meters, mini-map and objective reflect the game
bool testHud_StatusMiniMapAndScreen() {
    ASSERT_EQ(Hud::meter(1, 2), "[#-   ]");
    ASSERT_EQ(Hud::meter(5, 5), "[#####]");
    ASSERT_EQ(Hud::meter(0, 0), "[     ]");

    Gauntlet game(kSqlDir);
    for (int number = 1; number <= 12; ++number) {
        ASSERT_TRUE(clearCurrentRoom(game));
    }
    ASSERT_TRUE(walkTo(game, game.getCurrentLayout().ei_spawns[0]));
    std::vector<std::string> status = Hud::renderStatus(game);
    ASSERT_EQ(status[0], "Room 13: Ember Corridor III (13/100)");
    ASSERT_EQ(status[1], "Obstacles cleared 0/4, next ~ Misunderstanding");
    ASSERT_EQ(status[2].substr(0, 8), "Health [");
    ASSERT_EQ(status[3], "EI [#--  ] 1/3");
    ASSERT_EQ(status[4], "R  [     ] 0/0");
    ASSERT_EQ(status[5], "Load 1/10  Score 78"); // Rooms 1-12 cost, and score, 1 + 2 + ... + 12
    ASSERT_EQ(status[6].substr(0, status[6].find("Time")), "Turn " + std::to_string(game.getStats().getTurns()) + "  ");
    ASSERT_EQ(status[7], "Dash: ready ('r')");
    ASSERT_EQ(status[8], "Status: steady");

    std::vector<std::string> obstacles = Hud::renderObstacles(game);
    std::vector<std::string> expected_obstacles = {
        "In this room:",
        "  next ~ Misunderstanding       3 EI",
        "       ~ Misunderstanding       3 EI",
        "       ~ Misunderstanding       3 EI",
        "       z Apathy                 4 EI",
    };
    ASSERT_TRUE(obstacles == expected_obstacles);

    // Each room ahead shows its hardest obstacle's symbol
    std::vector<std::string> map = Hud::renderMiniMap(game);
    ASSERT_EQ(map.size(), 11u);
    ASSERT_EQ(map[1], "1   ##########");
    ASSERT_EQ(map[2], "11  ##@zzzzzzz");
    ASSERT_EQ(map[6], "51  %%%%%%====");
    ASSERT_EQ(map[10], "91  **********");

    std::vector<std::string> screen = Hud::renderScreen(game);
    ASSERT_EQ(screen[0], "+------^------+   Room 13: Ember Corridor III (13/100)");
    bool has_objective = false;
    for (const std::string& line : screen) {
        has_objective = has_objective ||
            line == "Objective: Collect 2 EI from the E and R tiles, then stand next to ~ Misunderstanding and press 'f'.";
    }
    ASSERT_TRUE(has_objective);
    return true;
}

// Test case: inventory, stats and help views toggle without changing the world
bool testHud_ViewsAndRebindingInHelp() {
    Gauntlet game(kSqlDir);
    ASSERT_TRUE(walkTo(game, game.getCurrentLayout().ei_spawns[0]));
    int turns = game.getStats().getTurns();

    ASSERT_TRUE(press(game, Command::ShowInventory));
    ASSERT_TRUE(game.getView() == GameView::Inventory);
    std::vector<std::string> inventory = Hud::renderScreen(game);
    ASSERT_EQ(inventory[0], "Inventory (load 1/10, 1 EI, 0 R)");
    ASSERT_EQ(inventory[1], "  E Mirror of Self-Awareness (+1 EI)");
    ASSERT_TRUE(press(game, Command::ShowInventory));
    ASSERT_TRUE(game.getView() == GameView::Room);

    ASSERT_TRUE(press(game, Command::ShowStats));
    std::vector<std::string> stats = Hud::renderScreen(game);
    ASSERT_EQ(stats[0], "Statistics");
    ASSERT_EQ(stats[1].substr(0, 10), "  Balanced");
    ASSERT_EQ(stats[4], "  EI collected 1, spent 0");
    ASSERT_TRUE(press(game, Command::MoveSouth)); // An action returns to the room view
    ASSERT_TRUE(game.getView() == GameView::Room);
    ASSERT_EQ(game.getStats().getTurns(), turns + 1);

    game.getKeyBindings().bind('9', Command::Clear);
    game.getKeyBindings().unbind('f');
    ASSERT_TRUE(press(game, Command::ShowHelp));
    bool listed = false;
    for (const std::string& line : Hud::renderScreen(game)) {
        listed = listed || line == "  9     clear";
    }
    ASSERT_TRUE(listed);
    ASSERT_TRUE(game.getObjective().find("press '9'") != std::string::npos);
    return true;
}

// BDD-style test: a different world loaded from its own tables plays through with the same rules
bool testGauntlet_PlaysCustomWorldFromData() {
    std::string directory = writeWorld("playable", customWorldTables());
    Gauntlet game(directory);
    ASSERT_EQ(game.getRoomCount(), 3);
    ASSERT_TRUE(game.getPlayer().getPosition() == Position(3, 4));
    std::vector<std::string> expected = {
        "+---^---+",
        "|...N...|",
        "|.E.....|",
        "|.......|",
        "|......:|",
        "|...@...|",
        "+-------+",
    };
    ASSERT_TRUE(Hud::renderRoom(game) == expected);

    // Exit descriptions come from the data too
    ASSERT_TRUE(clearCurrentRoom(game));
    ASSERT_TRUE(press(game, Command::Look));
    ASSERT_TRUE(press(game, Command::MoveSouth));
    ASSERT_EQ(game.getMessageLog().getMessages().back(), "To the south: Stairs lead down to the foyer.");

    while (game.getState() != GameState::Completed) {
        ASSERT_TRUE(clearCurrentRoom(game));
    }
    ASSERT_EQ(game.getStats().getObstaclesCleared(), 5);
    ASSERT_EQ(game.getStats().getObstaclesCleared(6), 1);
    ASSERT_EQ(game.getStats().getResilienceSpent(), 1);
    ASSERT_EQ(Hud::renderStatus(game)[0], "Room 3: Roof (3/3)");

    std::vector<std::string> stats = Hud::renderStats(game);
    bool listed = false;
    for (const std::string& line : stats) {
        listed = listed || line == "    6  W Washout                1";
    }
    ASSERT_TRUE(listed);
    std::filesystem::remove_all(directory);
    return true;
}

// BDD-style test: every move costs health; waiting, looking and views do not
bool testGauntlet_EveryMoveCostsHealth() {
    Gauntlet game(kSqlDir);
    ASSERT_EQ(game.getPlayer().getHealth(), 100);
    ASSERT_TRUE(press(game, Command::MoveNorthWest)); // (5,5)
    ASSERT_EQ(game.getPlayer().getHealth(), 99);
    ASSERT_EQ(game.getLastHealthChange(), -1);
    ASSERT_TRUE(press(game, Command::MoveWest));      // (4,5)
    ASSERT_EQ(game.getPlayer().getHealth(), 98);

    // Waiting, looking, views and blocked moves cost nothing
    ASSERT_TRUE(press(game, Command::Wait));
    ASSERT_TRUE(press(game, Command::Look));
    ASSERT_TRUE(press(game, Command::MoveNorth));
    ASSERT_TRUE(press(game, Command::ShowStats));
    ASSERT_TRUE(press(game, Command::ShowStats));
    ASSERT_TRUE(walkTo(game, {4, 6}));
    ASSERT_TRUE(!press(game, Command::MoveSouth)); // The south wall
    ASSERT_EQ(game.getPlayer().getHealth(), 97);
    ASSERT_EQ(game.getStats().getDamageTaken(), 3);
    ASSERT_TRUE(logContains(game, "Tip: every move costs health. Clearing an obstacle restores 10, plus 3 per type, so plan short routes."));
    return true;
}

// BDD-style test: hazards cost extra health, show on the map and can be looked at
bool testGauntlet_HazardsCostExtraHealth() {
    Gauntlet game(kSqlDir);
    ASSERT_TRUE(game.getCurrentLayout().hazardAt({6, 5}) != nullptr);
    ASSERT_TRUE(press(game, Command::Look));
    ASSERT_TRUE(press(game, Command::MoveNorth));
    ASSERT_EQ(game.getMessageLog().getMessages().back(), "To the north: Rumination Loop (-3 health). "
              "The same worried thought, circling. Stepping in costs extra health.");

    // WHEN the player steps onto it
    ASSERT_TRUE(press(game, Command::MoveNorth));
    // THEN it costs the move plus its damage, and the hit is reported with its amount
    ASSERT_EQ(game.getPlayer().getHealth(), 100 - 1 - 3);
    ASSERT_EQ(game.getLastHealthChange(), -4);
    ASSERT_EQ(game.getStats().getHazardsHit(), 1);
    GameEvent event;
    bool hit = false;
    while (game.pollEvent(event)) {
        hit = hit || (event.type == GameEventType::HazardHit && event.amount == 3);
    }
    ASSERT_TRUE(hit);
    ASSERT_TRUE(logContains(game, "Tip: ':' tiles are hazards that cost extra health. Walk around them when you can."));
    ASSERT_EQ(Hud::renderRoom(game)[6], "|......@......|");
    return true;
}

// BDD-style test: being shaken doubles the cost of moving until the player waits to breathe
bool testGauntlet_ShakenDoublesMovesUntilWaiting() {
    Gauntlet game(kSqlDir);
    ASSERT_TRUE(walkTo(game, {5, 1}));
    ASSERT_TRUE(!press(game, Command::Clear)); // No EI yet: struck and shaken
    ASSERT_EQ(game.getPlayer().getStatusTurns(StatusEffect::Shaken), Gauntlet::kShakenTurns);
    ASSERT_EQ(Hud::renderStatus(game)[8], "Status: Shaken (5 turns)");

    int health = game.getPlayer().getHealth();
    ASSERT_TRUE(press(game, Command::MoveSouth));
    ASSERT_EQ(game.getPlayer().getHealth(), health - 2);
    ASSERT_TRUE(press(game, Command::Wait));
    ASSERT_TRUE(!game.getPlayer().hasStatus(StatusEffect::Shaken));
    ASSERT_EQ(game.getMessageLog().getMessages().back(), "You take a slow breath and feel steady again.");
    ASSERT_TRUE(press(game, Command::MoveNorth));
    ASSERT_EQ(game.getPlayer().getHealth(), health - 3);

    // Shaken also wears off by itself
    ASSERT_TRUE(!press(game, Command::Clear));
    for (int turn = 0; turn < Gauntlet::kShakenTurns; ++turn) {
        ASSERT_TRUE(press(game, turn % 2 ? Command::MoveNorth : Command::MoveSouth));
    }
    ASSERT_TRUE(!game.getPlayer().hasStatus(StatusEffect::Shaken));
    ASSERT_TRUE(logContains(game, "The shakiness passes."));

    // A Grounded background is never shaken
    GameOptions grounded;
    grounded.background = Background::Grounded;
    Gauntlet calm(kSqlDir, grounded);
    ASSERT_TRUE(walkTo(calm, {5, 1}));
    ASSERT_TRUE(!press(calm, Command::Clear));
    ASSERT_TRUE(!calm.getPlayer().hasStatus(StatusEffect::Shaken));
    ASSERT_EQ(calm.getStats().getStrikesTaken(), 1);
    return true;
}

// BDD-style test: a dash covers two tiles for the cost of one move, then needs to recover
bool testGauntlet_DashCoversTwoTilesForOneMove() {
    Gauntlet game(kSqlDir);
    ASSERT_TRUE(press(game, Command::Dash));
    ASSERT_TRUE(game.getState() == GameState::Dashing);
    ASSERT_TRUE(press(game, Command::MoveNorthWest));
    ASSERT_TRUE(game.getPlayer().getPosition() == Position(4, 4));
    ASSERT_EQ(game.getPlayer().getHealth(), 99);
    ASSERT_EQ(game.getStats().getTurns(), 1);
    ASSERT_EQ(game.getStats().getDashes(), 1);
    ASSERT_EQ(game.getDashCooldown(), 5);
    ASSERT_EQ(Hud::renderStatus(game)[7], "Dash: 5 turns");

    // It cannot be used again until it recovers
    ASSERT_TRUE(!press(game, Command::Dash));
    ASSERT_EQ(game.getMessageLog().getMessages().back(), "You need 5 more turns before you can dash again.");
    for (int turn = 0; turn < 5; ++turn) {
        ASSERT_TRUE(press(game, Command::Wait));
    }
    ASSERT_EQ(game.getDashCooldown(), 0);

    // Dashing collects equipment on the way and stops at walls and obstacles
    ASSERT_TRUE(press(game, Command::Dash));
    ASSERT_TRUE(press(game, Command::MoveNorthWest));
    ASSERT_TRUE(game.getPlayer().getPosition() == Position(2, 2));
    ASSERT_EQ(game.getPlayer().getEmotionalIntelligence(), 1);
    for (int turn = 0; turn < 5; ++turn) {
        ASSERT_TRUE(press(game, Command::Wait));
    }
    ASSERT_TRUE(walkTo(game, {0, 1}));
    for (int turn = 0; turn < 5; ++turn) {
        ASSERT_TRUE(press(game, Command::Wait));
    }
    ASSERT_TRUE(press(game, Command::Dash));
    ASSERT_TRUE(press(game, Command::MoveNorth));
    ASSERT_TRUE(game.getPlayer().getPosition() == Position(0, 0)); // Only one tile before the wall

    // Swift runners recover in 2 turns
    GameOptions swift;
    swift.background = Background::Swift;
    Gauntlet quick(kSqlDir, swift);
    ASSERT_EQ(quick.getPlayer().getMaxHealth(), 80);
    ASSERT_TRUE(press(quick, Command::Dash));
    ASSERT_TRUE(press(quick, Command::MoveNorthWest));
    ASSERT_EQ(quick.getDashCooldown(), 2);
    return true;
}

// BDD-style test: at 0 health the player collapses back to the room's entry, and the ending shows it
bool testGauntlet_CollapseAndRisingAgainEnding() {
    std::string directory = writeWorld("collapse", customWorldTables());
    GameOptions swift;
    swift.background = Background::Swift;
    Gauntlet game(directory, swift);
    ASSERT_TRUE(clearCurrentRoom(game)); // Earns some score to lose

    int score = game.getPlayer().getScore();
    ASSERT_TRUE(walkUntilCollapse(game));
    ASSERT_EQ(game.getStats().getCollapses(), 1);
    ASSERT_EQ(game.getPlayer().getHealth(), 80);
    ASSERT_TRUE(game.getPlayer().getPosition() == game.getCurrentLayout().entry);
    ASSERT_EQ(game.getPlayer().getScore(), std::max(0, score - Gauntlet::kCollapseScorePenalty));
    ASSERT_TRUE(game.getState() == GameState::Playing);
    ASSERT_TRUE(drainEventsContain(game, GameEventType::Collapsed));

    while (game.getState() != GameState::Completed) {
        ASSERT_TRUE(clearCurrentRoom(game));
    }
    ASSERT_TRUE(game.getEnding() == Ending::RisingAgain);
    ASSERT_TRUE(!game.getStats().hasAchievement(Achievement::Unbroken));
    ASSERT_EQ(game.getMessageLog().getMessages().back(), "You have cleared all 3 rooms. Ending: Rising Again. "
              "You fell 1 time and got back up every time. That is what resilience means.");

    // A new run can begin once this one is over
    ASSERT_TRUE(press(game, Command::NewGame));
    ASSERT_EQ(game.getCurrentRoomNumber(), 1);
    ASSERT_EQ(game.getStats().getCollapses(), 0);
    ASSERT_TRUE(!press(game, Command::NewGame)); // Not in the middle of a run
    std::filesystem::remove_all(directory);
    return true;
}

// BDD-style test: on hardcore, collapsing ends the run and deletes its save
bool testGauntlet_HardcorePermadeath() {
    std::string world = writeWorld("hardcore", customWorldTables());
    std::string saves = tempDirectory("hardcore");
    GameOptions options = savingTo(saves);
    options.hardcore = true;
    Gauntlet game(world, options);

    // The first save picks a slot; later saves go to the same slot without asking
    ASSERT_TRUE(press(game, Command::Save));
    ASSERT_TRUE(game.handleKey('3'));
    ASSERT_TRUE(press(game, Command::MoveWest));
    ASSERT_TRUE(press(game, Command::Save));
    ASSERT_TRUE(game.getState() == GameState::Playing);
    ASSERT_TRUE(std::filesystem::exists(game.slotPath(3)));
    ASSERT_TRUE(!press(game, Command::Load));
    ASSERT_EQ(game.getMessageLog().getMessages().back(), "Hardcore runs cannot be reloaded.");

    ASSERT_TRUE(walkUntilCollapse(game));
    ASSERT_TRUE(game.getState() == GameState::Defeated);
    ASSERT_TRUE(game.getEnding() == Ending::Rest);
    ASSERT_TRUE(!std::filesystem::exists(game.slotPath(3)));
    ASSERT_TRUE(!press(game, Command::MoveNorth));
    ASSERT_TRUE(!press(game, Command::Save));
    ASSERT_EQ(game.getObjective(), "The run is over. Press 'N' to begin a new one.");

    ASSERT_TRUE(press(game, Command::NewGame));
    ASSERT_TRUE(game.getState() == GameState::Playing);
    ASSERT_EQ(game.getPlayer().getHealth(), 100);
    ASSERT_TRUE(game.getOptions().hardcore);
    std::filesystem::remove_all(world);
    std::filesystem::remove_all(saves);
    return true;
}

// BDD-style test: three save slots are independent, and a slot prompt can be cancelled
bool testGauntlet_SaveSlots() {
    std::string saves = tempDirectory("slots");
    Gauntlet game(kSqlDir, savingTo(saves));
    ASSERT_TRUE(clearCurrentRoom(game));
    ASSERT_TRUE(press(game, Command::Save));
    ASSERT_TRUE(game.handleKey('1'));
    ASSERT_TRUE(clearCurrentRoom(game));
    ASSERT_TRUE(press(game, Command::Save));
    ASSERT_TRUE(game.handleKey('3'));

    ASSERT_TRUE(press(game, Command::Save));
    ASSERT_TRUE(!game.handleKey('x')); // Cancels
    ASSERT_TRUE(game.getState() == GameState::Playing);
    ASSERT_TRUE(!std::filesystem::exists(game.slotPath(2)));

    ASSERT_TRUE(press(game, Command::Load));
    ASSERT_TRUE(game.handleKey('1'));
    ASSERT_EQ(game.getCurrentRoomNumber(), 2);
    ASSERT_TRUE(press(game, Command::Load));
    ASSERT_TRUE(game.handleKey('3'));
    ASSERT_EQ(game.getCurrentRoomNumber(), 3);
    ASSERT_EQ(game.getMessageLog().getMessages().back(), "Slot 3 loaded. You are in Room 3: Hall of Echoes III.");
    std::filesystem::remove_all(saves);
    return true;
}

// Test case: the speedrun timer counts play time, stops while paused, and is saved
bool testGauntlet_SpeedrunTimer() {
    Gauntlet game(kSqlDir);
    long long now = 1000;
    game.setClock([&now]() { return now; });
    now += 1500;
    ASSERT_TRUE(press(game, Command::Wait));
    ASSERT_EQ(game.getElapsedMilliseconds(), 1500);
    ASSERT_TRUE(press(game, Command::Pause));
    now += 60000; // Paused: does not count
    ASSERT_EQ(game.getElapsedMilliseconds(), 1500);
    ASSERT_TRUE(press(game, Command::Pause));
    now += 500;
    ASSERT_EQ(game.getElapsedMilliseconds(), 2000);
    ASSERT_EQ(Hud::formatTime(game.getElapsedMilliseconds()), "0:02");
    ASSERT_EQ(Hud::formatTime(3723000), "1:02:03");

    std::stringstream saved;
    ASSERT_TRUE(game.save(saved));
    Gauntlet other(kSqlDir);
    other.setClock([&now]() { return now; });
    ASSERT_TRUE(other.load(saved));
    ASSERT_EQ(other.getElapsedMilliseconds(), 2000);
    return true;
}

// Test case: the bestiary fills in as obstacle types are met
bool testGauntlet_BestiaryFillsIn() {
    Gauntlet game(kSqlDir);
    ASSERT_TRUE(game.getStats().hasSeen(1));
    ASSERT_TRUE(!game.getStats().hasSeen(2));
    ASSERT_TRUE(press(game, Command::ShowBestiary));
    ASSERT_TRUE(game.getView() == GameView::Bestiary);
    std::vector<std::string> lines = Hud::renderScreen(game);
    ASSERT_EQ(lines[1], "  1  ? Self-Doubt  needs 1 EI, strikes for 2, cleared 0");
    ASSERT_EQ(lines[2], "       A fog of 'I can't do this' hangs across the doorway.");
    ASSERT_EQ(lines[3], "  2  ? ???  (not met yet)");
    ASSERT_TRUE(press(game, Command::ShowBestiary));

    ASSERT_TRUE(clearCurrentRoom(game));
    ASSERT_TRUE(clearCurrentRoom(game));
    ASSERT_TRUE(game.getStats().hasSeen(2)); // Room 3 holds an Impulsive Anger
    ASSERT_EQ(Hud::renderBestiary(game)[3], "  2  ! Impulsive Anger  needs 2 EI, strikes for 4, cleared 0");
    return true;
}

// Test case: tutorial tips appear once each, and not at all when turned off
bool testGauntlet_TutorialTips() {
    Gauntlet game(kSqlDir);
    ASSERT_TRUE(press(game, Command::MoveNorthWest));
    ASSERT_TRUE(press(game, Command::MoveSouthEast));
    int tips = 0;
    GameEvent event;
    while (game.pollEvent(event)) {
        tips += event.type == GameEventType::Tip ? 1 : 0;
    }
    ASSERT_EQ(tips, 1);

    GameOptions quiet;
    quiet.tutorial = false;
    Gauntlet silent(kSqlDir, quiet);
    ASSERT_TRUE(press(silent, Command::MoveNorth)); // A hazard, and the first move
    ASSERT_TRUE(!drainEventsContain(silent, GameEventType::Tip));
    return true;
}

// Test case: the quit key asks the frontend to stop
bool testGauntlet_Quit() {
    Gauntlet game(kSqlDir);
    ASSERT_TRUE(!game.isQuitRequested());
    ASSERT_TRUE(game.handleKey('Q'));
    ASSERT_TRUE(game.isQuitRequested());
    ASSERT_TRUE(!game.handleKey('?')); // Unbound keys do nothing
    return true;
}

// Function to register all gauntlet tests with the runner
void registerGauntletTests(TestRunner& runner) {
    runner.addTest("testKeyBindings_DefaultsRebindAndLoad", testKeyBindings_DefaultsRebindAndLoad);
    runner.addTest("testGauntlet_StartsInFirstRoom", testGauntlet_StartsInFirstRoom);
    runner.addTest("testGauntlet_DiagonalMovementAndWalls", testGauntlet_DiagonalMovementAndWalls);
    runner.addTest("testGauntlet_CollectsEquipmentFromSpawnPoints", testGauntlet_CollectsEquipmentFromSpawnPoints);
    runner.addTest("testGauntlet_ObstacleRules", testGauntlet_ObstacleRules);
    runner.addTest("testGauntlet_ObstaclesClearInOrderAndRefillSpawns", testGauntlet_ObstaclesClearInOrderAndRefillSpawns);
    runner.addTest("testGauntlet_LookWithoutMoving", testGauntlet_LookWithoutMoving);
    runner.addTest("testGauntlet_WaitAndPause", testGauntlet_WaitAndPause);
    runner.addTest("testGauntlet_BacktrackKeepsWorldState", testGauntlet_BacktrackKeepsWorldState);
    runner.addTest("testGauntlet_PlayThroughAllHundredRoomsWithKeys", testGauntlet_PlayThroughAllHundredRoomsWithKeys);
    runner.addTest("testGauntlet_SaveAndLoadRoundTrip", testGauntlet_SaveAndLoadRoundTrip);
    runner.addTest("testGauntlet_LoadRejectsBadData", testGauntlet_LoadRejectsBadData);
    runner.addTest("testGauntlet_EventQueueAndMessageLog", testGauntlet_EventQueueAndMessageLog);
    runner.addTest("testHud_RenderRoom", testHud_RenderRoom);
    runner.addTest("testHud_StatusMiniMapAndScreen", testHud_StatusMiniMapAndScreen);
    runner.addTest("testHud_ViewsAndRebindingInHelp", testHud_ViewsAndRebindingInHelp);
    runner.addTest("testGauntlet_PlaysCustomWorldFromData", testGauntlet_PlaysCustomWorldFromData);
    runner.addTest("testGauntlet_EveryMoveCostsHealth", testGauntlet_EveryMoveCostsHealth);
    runner.addTest("testGauntlet_HazardsCostExtraHealth", testGauntlet_HazardsCostExtraHealth);
    runner.addTest("testGauntlet_ShakenDoublesMovesUntilWaiting", testGauntlet_ShakenDoublesMovesUntilWaiting);
    runner.addTest("testGauntlet_DashCoversTwoTilesForOneMove", testGauntlet_DashCoversTwoTilesForOneMove);
    runner.addTest("testGauntlet_CollapseAndRisingAgainEnding", testGauntlet_CollapseAndRisingAgainEnding);
    runner.addTest("testGauntlet_HardcorePermadeath", testGauntlet_HardcorePermadeath);
    runner.addTest("testGauntlet_SaveSlots", testGauntlet_SaveSlots);
    runner.addTest("testGauntlet_SpeedrunTimer", testGauntlet_SpeedrunTimer);
    runner.addTest("testGauntlet_BestiaryFillsIn", testGauntlet_BestiaryFillsIn);
    runner.addTest("testGauntlet_TutorialTips", testGauntlet_TutorialTips);
    runner.addTest("testGauntlet_Quit", testGauntlet_Quit);
}
