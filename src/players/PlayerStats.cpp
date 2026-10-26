#include "PlayerStats.h"
#include <numeric>

namespace {

struct AchievementInfo {
    const char* name;
    const char* description;
};

const AchievementInfo kAchievementInfo[static_cast<int>(Achievement::Count)] = {
    {"First Step", "Clear your first obstacle."},
    {"Emotionally Intelligent", "Clear an obstacle that demands all five EI."},
    {"Bouncing Back", "Clear your first obstacle that demands resilience."},
    {"Halfway There", "Clear half of all obstacles."},
    {"Unbreakable", "Clear every obstacle."},
    {"Unbroken", "Clear every obstacle without collapsing once."},
};

unsigned bit(Achievement achievement) {
    return 1u << static_cast<unsigned>(achievement);
}

} // namespace

PlayerStats::PlayerStats()
    : turns(0), steps(0), waits(0), blocked_moves(0), backtracks(0), furthest_room(0),
      ei_collected(0), r_collected(0), ei_spent(0), r_spent(0), achievements(0), damage_taken(0),
      healing_received(0), hazards_hit(0), strikes_taken(0), dashes(0), collapses(0), seen_types(0),
      tips_shown(0), elapsed_ms(0) {
    cleared_by_type.fill(0);
}

void PlayerStats::recordTurn() { turns++; }
void PlayerStats::recordStep() { steps++; }
void PlayerStats::recordWait() { waits++; }
void PlayerStats::recordBlockedMove() { blocked_moves++; }
void PlayerStats::recordBacktrack() { backtracks++; }
void PlayerStats::recordDamage(int amount) { damage_taken += amount; }
void PlayerStats::recordHealing(int amount) { healing_received += amount; }
void PlayerStats::recordHazard() { hazards_hit++; }
void PlayerStats::recordStrike() { strikes_taken++; }
void PlayerStats::recordDash() { dashes++; }
void PlayerStats::recordCollapse() { collapses++; }
void PlayerStats::addElapsed(long long milliseconds) { elapsed_ms += milliseconds; }

void PlayerStats::recordSeen(int obstacle_type) {
    if (Obstacle::isValidType(obstacle_type)) {
        seen_types |= 1u << obstacle_type;
    }
}

bool PlayerStats::markTipShown(int tip) {
    unsigned flag = 1u << tip;
    bool first = (tips_shown & flag) == 0;
    tips_shown |= flag;
    return first;
}

void PlayerStats::recordRoomReached(int room_number) {
    if (room_number > furthest_room) {
        furthest_room = room_number;
    }
}

void PlayerStats::recordPickup(EquipmentKind kind) {
    if (kind == EquipmentKind::EmotionalIntelligence) {
        ei_collected++;
    } else {
        r_collected++;
    }
}

bool PlayerStats::unlock(Achievement achievement) {
    if (hasAchievement(achievement)) {
        return false;
    }
    achievements |= bit(achievement);
    return true;
}

std::vector<Achievement> PlayerStats::recordClear(int type, int ei, int r, int total_obstacles) {
    cleared_by_type[type]++;
    ei_spent += ei;
    r_spent += r;

    std::vector<Achievement> unlocked;
    int cleared = getObstaclesCleared();
    auto check = [&](bool condition, Achievement achievement) {
        if (condition && unlock(achievement)) {
            unlocked.push_back(achievement);
        }
    };
    check(cleared >= 1, Achievement::FirstStep);
    check(ei >= 5, Achievement::EmotionallyIntelligent);
    check(r >= 1, Achievement::BouncingBack);
    check(cleared * 2 >= total_obstacles, Achievement::HalfwayThere);
    check(cleared >= total_obstacles, Achievement::Unbreakable);
    check(cleared >= total_obstacles && collapses == 0, Achievement::Unbroken);
    return unlocked;
}

int PlayerStats::getTurns() const { return turns; }
int PlayerStats::getSteps() const { return steps; }
int PlayerStats::getWaits() const { return waits; }
int PlayerStats::getBlockedMoves() const { return blocked_moves; }
int PlayerStats::getBacktracks() const { return backtracks; }
int PlayerStats::getFurthestRoom() const { return furthest_room; }
int PlayerStats::getEmotionalIntelligenceCollected() const { return ei_collected; }
int PlayerStats::getResilienceCollected() const { return r_collected; }
int PlayerStats::getEmotionalIntelligenceSpent() const { return ei_spent; }
int PlayerStats::getResilienceSpent() const { return r_spent; }
int PlayerStats::getDamageTaken() const { return damage_taken; }
int PlayerStats::getHealingReceived() const { return healing_received; }
int PlayerStats::getHazardsHit() const { return hazards_hit; }
int PlayerStats::getStrikesTaken() const { return strikes_taken; }
int PlayerStats::getDashes() const { return dashes; }
int PlayerStats::getCollapses() const { return collapses; }
long long PlayerStats::getElapsedMilliseconds() const { return elapsed_ms; }

bool PlayerStats::hasSeen(int obstacle_type) const {
    return Obstacle::isValidType(obstacle_type) && (seen_types & (1u << obstacle_type)) != 0;
}

int PlayerStats::getObstaclesCleared() const {
    return std::accumulate(cleared_by_type.begin(), cleared_by_type.end(), 0);
}

int PlayerStats::getObstaclesCleared(int type) const {
    if (type < Obstacle::kMinType || type > Obstacle::kMaxType) {
        return 0;
    }
    return cleared_by_type[type];
}

bool PlayerStats::hasAchievement(Achievement achievement) const {
    return (achievements & bit(achievement)) != 0;
}

std::vector<Achievement> PlayerStats::getAchievements() const {
    std::vector<Achievement> unlocked;
    for (int i = 0; i < static_cast<int>(Achievement::Count); ++i) {
        if (hasAchievement(static_cast<Achievement>(i))) {
            unlocked.push_back(static_cast<Achievement>(i));
        }
    }
    return unlocked;
}

std::string PlayerStats::achievementName(Achievement achievement) {
    return kAchievementInfo[static_cast<int>(achievement)].name;
}

std::string PlayerStats::achievementDescription(Achievement achievement) {
    return kAchievementInfo[static_cast<int>(achievement)].description;
}

void PlayerStats::write(std::ostream& out) const {
    out << turns << ' ' << steps << ' ' << waits << ' ' << blocked_moves << ' ' << backtracks << ' '
        << furthest_room << ' ' << ei_collected << ' ' << r_collected << ' ' << ei_spent << ' ' << r_spent;
    for (int type = Obstacle::kMinType; type <= Obstacle::kMaxType; ++type) {
        out << ' ' << cleared_by_type[type];
    }
    out << ' ' << achievements << ' ' << damage_taken << ' ' << healing_received << ' ' << hazards_hit << ' '
        << strikes_taken << ' ' << dashes << ' ' << collapses << ' ' << seen_types << ' ' << tips_shown << ' '
        << elapsed_ms;
}

bool PlayerStats::read(std::istream& in) {
    PlayerStats loaded;
    in >> loaded.turns >> loaded.steps >> loaded.waits >> loaded.blocked_moves >> loaded.backtracks
       >> loaded.furthest_room >> loaded.ei_collected >> loaded.r_collected >> loaded.ei_spent >> loaded.r_spent;
    for (int type = Obstacle::kMinType; type <= Obstacle::kMaxType; ++type) {
        in >> loaded.cleared_by_type[type];
    }
    in >> loaded.achievements >> loaded.damage_taken >> loaded.healing_received >> loaded.hazards_hit
       >> loaded.strikes_taken >> loaded.dashes >> loaded.collapses >> loaded.seen_types >> loaded.tips_shown
       >> loaded.elapsed_ms;
    if (!in) {
        return false;
    }
    *this = loaded;
    return true;
}
