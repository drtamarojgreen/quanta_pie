#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include "Tool.h"
#include <string>

/**
 * @enum EquipmentKind
 * @brief The inner capacity a piece of equipment strengthens.
 */
enum class EquipmentKind {
    EmotionalIntelligence,
    Resilience
};

/**
 * @class Equipment
 * @brief A tool that grants Emotional Intelligence (EI) or Resilience (R) while carried.
 *
 * Equipment is picked up and dropped like any other tool. A player's EI and R
 * are the sum of the points of the equipment they are currently carrying.
 * Clearing an obstacle consumes the equipment spent on it. Equipment waits unseen until it
 * is spawned onto its tile.
 */
class Equipment : public Tool {
public:
    Equipment(int id, const std::string& name, const std::string& description, int initial_room_id,
              EquipmentKind kind, int points = 1);

    EquipmentKind getKind() const;
    int getPoints() const;
    bool isConsumed() const;
    void consume();
    bool isSpawned() const;
    void spawn();

private:
    EquipmentKind kind;
    int points;
    bool consumed;
    bool spawned;
};

#endif // EQUIPMENT_H
