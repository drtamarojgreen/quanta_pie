#include "Equipment.h"

Equipment::Equipment(int id, const std::string& name, const std::string& description, int initial_room_id,
                     EquipmentKind kind, int points)
    : Tool(id, name, description, initial_room_id), kind(kind), points(points), consumed(false), spawned(false) {
}

EquipmentKind Equipment::getKind() const {
    return kind;
}

int Equipment::getPoints() const {
    return points;
}

bool Equipment::isConsumed() const {
    return consumed;
}

void Equipment::consume() {
    consumed = true;
}

bool Equipment::isSpawned() const {
    return spawned;
}

void Equipment::spawn() {
    spawned = true;
}
