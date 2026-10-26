#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "RoomObject.h"
#include <string>

/**
 * @class Obstacle
 * @brief An emotional obstacle that blocks an exit until it is cleared.
 *
 * There are ten obstacle types of increasing difficulty. Types 1-5 require
 * 1-5 Emotional Intelligence (EI). Types 6-10 require 5 EI plus 1-5 Resilience (R).
 * Names, descriptions and map symbols come from the world data (sql/gauntlet_obstacles.csv);
 * each type has its own symbol so it can be recognised on the map.
 */
class Obstacle : public RoomObject {
public:
    static constexpr int kMinType = 1;
    static constexpr int kMaxType = 10;

    /**
     * @throws std::invalid_argument if type is outside [kMinType, kMaxType].
     */
    Obstacle(int id, int type, int room_id, const std::string& name, const std::string& description,
             char symbol = 'X', const std::string& blocked_direction = "north");

    int getType() const;
    char getSymbol() const;
    std::string getLabel() const; // Symbol and name, e.g. "? Self-Doubt"
    int getRequiredEmotionalIntelligence() const;
    int getRequiredResilience() const;
    const std::string& getBlockedDirection() const;
    bool blocks(const std::string& direction) const;
    bool meetsRequirements(int emotional_intelligence, int resilience) const;

    bool isCleared() const;
    void clear();

    static int requiredEmotionalIntelligenceFor(int type);
    static int requiredResilienceFor(int type);
    static bool isValidType(int type);

private:
    int type;
    char symbol;
    std::string blocked_direction;
    bool cleared;
};

#endif // OBSTACLE_H
