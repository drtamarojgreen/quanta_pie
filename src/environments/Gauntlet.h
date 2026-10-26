#ifndef GAUNTLET_H
#define GAUNTLET_H

#include <deque>
#include <functional>
#include <istream>
#include <memory>
#include <ostream>
#include <string>

#include "Environment.h"
#include "MessageLog.h"
#include "RoomLayout.h"
#include "WorldData.h"
#include "../players/Background.h"
#include "../players/KeyBindings.h"
#include "../players/Player.h"
#include "../players/PlayerStats.h"

/**
 * @enum GameState
 * @brief The engine's state machine. Looking and Dashing wait for a direction;
 *        the slot states wait for a slot number.
 */
enum class GameState {
    Playing,
    Looking,
    Dashing,
    ChoosingSaveSlot,
    ChoosingLoadSlot,
    Paused,
    Completed,
    Defeated // Hardcore only: the run is over
};

/**
 * @enum GameView
 * @brief Which screen the HUD should draw. Views never change the game world.
 */
enum class GameView {
    Room,
    Inventory,
    Stats,
    Bestiary,
    Help
};

/**
 * @enum Ending
 * @brief How a run finished, chosen by how it was played.
 */
enum class Ending {
    None,        // Still playing
    Unbroken,    // Every room cleared without collapsing
    RisingAgain, // Every room cleared after collapsing at least once
    Rest         // Collapsed on hardcore
};

enum class GameEventType {
    RoomEntered,
    PickedUp,
    InventoryFull,
    Blocked,
    Cleared,
    CannotClear,
    EquipmentSpawned,
    Struck,      // An obstacle struck back; amount is the health lost
    HazardHit,   // amount is the health lost
    Healed,      // amount is the health gained
    StatusChanged,
    Dashed,
    Collapsed,
    Defeated,
    Looked,
    Waited,
    Paused,
    Resumed,
    Prompt,
    Ignored,
    Saved,
    SaveFailed,
    Loaded,
    LoadFailed,
    AchievementUnlocked,
    Tip,
    Completed
};

struct GameEvent {
    GameEventType type;
    std::string message;
    int amount = 0;
};

/**
 * @struct GameOptions
 * @brief Choices made before a run starts (normally from config.txt or the welcome screen).
 */
struct GameOptions {
    std::string save_directory = ".";
    Background background = Background::Balanced;
    bool hardcore = false; // Collapsing ends the run and deletes its save
    bool tutorial = true;  // Show one-time tips as new mechanics come up
};

/**
 * @class Gauntlet
 * @brief Runs a playthrough of an Environment: input, rules, events, objectives and save/load.
 *
 * The world comes from the CSV tables in a data directory (normally "sql"); it is read once
 * and rebuilt from memory for a new game or a loaded save.
 *
 * Input arrives as keys (through KeyBindings) or Commands. Every notable outcome is
 * queued as a GameEvent for frontends and also written to the MessageLog for the HUD.
 * A room's obstacles are the enemies: they are cleared one at a time, in order, and the
 * north door opens when the last one is gone. Every move costs health; clearing an
 * obstacle restores some, and attempting one unprepared makes it strike back. At 0 health
 * the player collapses: back to the room's entry with full health, or the end of the run
 * on hardcore.
 */
class Gauntlet {
public:
    static const std::string kDefaultDataDirectory;
    static constexpr int kSaveSlots = 3;
    static constexpr int kStepCost = 1;        // Health per move
    static constexpr int kShakenStepCost = 2;  // Health per move while shaken
    static constexpr int kHealBase = 10;       // Clearing a type-t obstacle heals kHealBase + kHealPerType * t
    static constexpr int kHealPerType = 3;
    static constexpr int kStrikePerType = 2;   // An unprepared attempt at a type-t obstacle costs 2t health
    static constexpr int kShakenTurns = 5;
    static constexpr int kDashDistance = 2;
    static constexpr int kCollapseScorePenalty = 25;

    using Clock = std::function<long long()>; // Milliseconds since any fixed point

    /**
     * @throws WorldDataError if the data directory's tables are missing or invalid.
     */
    explicit Gauntlet(const std::string& data_directory = kDefaultDataDirectory,
                      const GameOptions& options = GameOptions());
    explicit Gauntlet(const WorldData& world, const GameOptions& options = GameOptions());

    void newGame(); // With the current options
    void newGame(const GameOptions& options);
    const GameOptions& getOptions() const;

    bool handleKey(char key);       // Returns true if the key's command took effect
    bool execute(Command command);  // Returns true if the command took effect

    GameState getState() const;
    GameView getView() const;
    bool isQuitRequested() const;
    Ending getEnding() const;
    static std::string endingTitle(Ending ending);
    std::string getEndingText() const;

    const Environment& getEnvironment() const;
    const Player& getPlayer() const;
    const PlayerStats& getStats() const;
    const MessageLog& getMessageLog() const;
    KeyBindings& getKeyBindings();
    const KeyBindings& getKeyBindings() const;

    int getRoomCount() const;
    int getCurrentRoomNumber() const;
    std::string getRoomTitle(int room_number) const; // e.g. "Room 13: Ember Corridor III"
    Room* getCurrentRoom() const;
    const RoomLayout& getCurrentLayout() const;
    Obstacle* getActiveObstacle() const; // The current room's next obstacle; nullptr once it is clear

    int getDashCooldown() const;      // Turns until the dash can be used; 0 when ready
    int getLastHealthChange() const;  // Health gained (+) or lost (-) by the last action
    long long getElapsedMilliseconds() const; // Play time, not counting pauses
    void setClock(Clock clock);       // For tests; defaults to a steady wall clock

    std::string describe(Direction direction) const; // What lies one tile away, without moving
    std::string getObjective() const;                // The current goal, in words
    std::string keyHint(Command command) const;      // e.g. "'f'", for messages

    bool pollEvent(GameEvent& event); // Pops the oldest pending event; false when none are left

    std::string slotPath(int slot) const; // The save file for slot 1..kSaveSlots
    int getCurrentSlot() const;           // The slot last saved to or loaded from; 0 if none
    bool saveToSlot(int slot);
    bool loadFromSlot(int slot);
    bool save(std::ostream& out) const;
    bool load(std::istream& in); // Leaves the game untouched and returns false if the data is invalid
    bool saveToFile(const std::string& path) const;
    bool loadFromFile(const std::string& path);

private:
    static constexpr size_t kMaxPendingEvents = 256;

    WorldData world;
    int room_count;
    GameOptions options;
    std::unique_ptr<Environment> environment;
    std::unique_ptr<Player> player;
    KeyBindings bindings;
    PlayerStats stats;
    MessageLog log;
    std::deque<GameEvent> events;
    GameState state;
    GameState resume_state; // Where a slot prompt returns to
    GameView view;
    Ending ending;
    int current_room;
    int current_slot;
    int dash_cooldown;
    int health_change;
    bool quit_requested;
    Clock clock;
    long long last_tick;

    bool step(Direction direction);
    bool dash(Direction direction);
    bool changeRoom(const std::string& exit);
    void moveTo(const Position& position);
    bool blocked(const std::string& message);
    Obstacle* obstacleAt(const Position& position) const; // An uncleared obstacle on that tile, if any
    void collectAt(const Position& position);
    bool wait();
    bool clearObstacle();
    void payMove();
    void loseHealth(int amount);
    void endTurn();
    void checkCollapse();
    void finish();
    bool chooseSlot(int slot);
    void enterRoom(int room_number, const Position& arrival);
    void tip(int which);
    bool timerRunning() const;
    void updateTimer();
    void emit(GameEventType type, const std::string& message, int amount = 0);
};

#endif // GAUNTLET_H
