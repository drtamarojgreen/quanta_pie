#ifndef PLAYER_STATS_H
#define PLAYER_STATS_H

#include <array>
#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include "../objects/Equipment.h"
#include "../objects/Obstacle.h"

/**
 * @enum Achievement
 * @brief Milestones unlocked while working through the rooms.
 */
enum class Achievement {
    FirstStep,              // Clear the first obstacle
    EmotionallyIntelligent, // Clear an obstacle needing the full 5 EI
    BouncingBack,           // Clear the first obstacle that needs resilience
    HalfwayThere,           // Clear half of all obstacles
    Unbreakable,            // Clear every obstacle
    Unbroken,               // Clear every obstacle without collapsing once
    Count
};

/**
 * @class PlayerStats
 * @brief Running statistics and unlocked achievements for one playthrough.
 */
class PlayerStats {
public:
    PlayerStats();

    void recordTurn();
    void recordStep();
    void recordWait();
    void recordBlockedMove();
    void recordBacktrack();
    void recordRoomReached(int room_number);
    void recordPickup(EquipmentKind kind);
    void recordDamage(int amount);
    void recordHealing(int amount);
    void recordHazard();
    void recordStrike(); // An obstacle struck back at an unprepared attempt
    void recordDash();
    void recordCollapse();
    void recordSeen(int obstacle_type); // For the bestiary
    bool markTipShown(int tip); // Returns false if the tip was already shown
    void addElapsed(long long milliseconds);

    /**
     * @brief Records a cleared obstacle.
     * @return The achievements this clear unlocked, if any.
     */
    std::vector<Achievement> recordClear(int type, int ei_spent, int r_spent, int total_obstacles);

    int getTurns() const;
    int getSteps() const;
    int getWaits() const;
    int getBlockedMoves() const;
    int getBacktracks() const;
    int getFurthestRoom() const;
    int getEmotionalIntelligenceCollected() const;
    int getResilienceCollected() const;
    int getEmotionalIntelligenceSpent() const;
    int getResilienceSpent() const;
    int getObstaclesCleared() const;
    int getObstaclesCleared(int type) const;
    int getDamageTaken() const;
    int getHealingReceived() const;
    int getHazardsHit() const;
    int getStrikesTaken() const;
    int getDashes() const;
    int getCollapses() const;
    bool hasSeen(int obstacle_type) const;
    long long getElapsedMilliseconds() const;

    bool hasAchievement(Achievement achievement) const;
    std::vector<Achievement> getAchievements() const;
    static std::string achievementName(Achievement achievement);
    static std::string achievementDescription(Achievement achievement);

    void write(std::ostream& out) const;
    bool read(std::istream& in);

private:
    int turns;
    int steps;
    int waits;
    int blocked_moves;
    int backtracks;
    int furthest_room;
    int ei_collected;
    int r_collected;
    int ei_spent;
    int r_spent;
    std::array<int, Obstacle::kMaxType + 1> cleared_by_type; // Index 0 unused
    unsigned achievements; // Bit per Achievement
    int damage_taken;
    int healing_received;
    int hazards_hit;
    int strikes_taken;
    int dashes;
    int collapses;
    unsigned seen_types; // Bit per obstacle type
    unsigned tips_shown; // Bit per tutorial tip
    long long elapsed_ms;

    bool unlock(Achievement achievement);
};

#endif // PLAYER_STATS_H
