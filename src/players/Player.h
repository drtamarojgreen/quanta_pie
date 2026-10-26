#ifndef PLAYER_H
#define PLAYER_H

#include <map>
#include <string>
#include <vector>
#include "../objects/Tool.h"
#include "../objects/Equipment.h"
#include "../objects/Position.h"

class Room; // Forward declaration for Room
class Obstacle;

/**
 * @enum StatusEffect
 * @brief Temporary conditions on the player.
 */
enum class StatusEffect {
    Shaken // Each move costs double health; a slow breath (waiting) cures it
};

class Player {
public:
    static constexpr int kDefaultCarryCapacity = 10; // Enough for the hardest obstacle: 5 EI + 5 R
    static constexpr int kDefaultMaxHealth = 100;

    Player(int id, const std::string& name, const std::string& joinDate, Room* startingRoom);
    Player(Room* startingRoom);

    bool move(const std::string& direction); // Returns false if there is no exit or an obstacle blocks it
    void setCurrentRoom(Room* room);
    Room* getCurrentRoom() const;
    int getID() const;
    std::string getName() const;
    std::string getJoinDate() const;
    std::string getRepresentation() const;
    int getScore() const;
    void incrementScore(int amount = 1);

    // Tile inside the current room; invalid when the player is not on a grid
    Position getPosition() const;
    void setPosition(const Position& position);

    // Inventory methods
    bool takeTool(Tool* tool); // Returns false if the tool would exceed the carry capacity
    void dropTool(Tool* tool);
    const std::vector<Tool*>& getTools() const;
    int getCarryWeight() const;
    int getCarryCapacity() const;
    void setCarryCapacity(int capacity);

    // Emotional equipment: EI and R are the totals of the equipment currently carried
    int getEmotionalIntelligence() const;
    int getResilience() const;
    int pickUpEquipment(); // Takes all equipment in the current room that fits; returns how many were taken

    // Health: moves and obstacles drain it; reaching 0 means the player collapses
    int getHealth() const;
    int getMaxHealth() const;
    void setMaxHealth(int max_health); // Also fills health to the new maximum
    void restoreHealth();
    int damage(int amount); // Returns the health actually lost
    int heal(int amount);   // Returns the health actually gained, up to the maximum
    bool isDefeated() const;

    // Status effects last a number of turns
    void addStatus(StatusEffect effect, int turns);
    void removeStatus(StatusEffect effect);
    bool hasStatus(StatusEffect effect) const;
    int getStatusTurns(StatusEffect effect) const; // 0 if not affected
    void tickStatuses(); // Counts every effect down by one turn, removing those that run out

    // Obstacles
    bool canClear(const Obstacle* obstacle) const; // Needs enough EI and R, and to be adjacent if both are placed
    bool clearObstacle(Obstacle* obstacle); // Spends the required EI and R, then clears and removes the obstacle
    Obstacle* getBlockingObstacle(const std::string& direction) const;
    int getObstaclesCleared() const;

private:
    int id;
    std::string name;
    std::string joinDate;
    Room* currentRoom;
    int score; // Added for tracking player score
    std::vector<Tool*> tools; // Player's inventory of tools
    int obstaclesCleared;
    Position position;
    int carryCapacity;
    int maxHealth;
    int health;
    std::map<StatusEffect, int> statuses;
    // Cached totals so HUD and obstacle checks don't rescan the inventory
    int carryWeight;
    int emotionalIntelligence;
    int resilience;

    void recalculateInventory();
    void spendEquipment(EquipmentKind kind, int points);
};

#endif // PLAYER_H
