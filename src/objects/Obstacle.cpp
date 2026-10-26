#include "Obstacle.h"
#include <algorithm>
#include <stdexcept>

namespace {

int validated(int type) {
    if (!Obstacle::isValidType(type)) {
        throw std::invalid_argument("Obstacle type must be between 1 and 10");
    }
    return type;
}

} // namespace

Obstacle::Obstacle(int id, int type, int room_id, const std::string& name, const std::string& description,
                   char symbol, const std::string& blocked_direction)
    : RoomObject(id, name, description, room_id),
      type(validated(type)), symbol(symbol), blocked_direction(blocked_direction), cleared(false) {
}

int Obstacle::getType() const {
    return type;
}

char Obstacle::getSymbol() const {
    return symbol;
}

std::string Obstacle::getLabel() const {
    return std::string(1, symbol) + " " + getName();
}

int Obstacle::getRequiredEmotionalIntelligence() const {
    return requiredEmotionalIntelligenceFor(type);
}

int Obstacle::getRequiredResilience() const {
    return requiredResilienceFor(type);
}

const std::string& Obstacle::getBlockedDirection() const {
    return blocked_direction;
}

bool Obstacle::blocks(const std::string& direction) const {
    return !cleared && direction == blocked_direction;
}

bool Obstacle::meetsRequirements(int emotional_intelligence, int resilience) const {
    return emotional_intelligence >= getRequiredEmotionalIntelligence() &&
           resilience >= getRequiredResilience();
}

bool Obstacle::isCleared() const {
    return cleared;
}

void Obstacle::clear() {
    cleared = true;
}

int Obstacle::requiredEmotionalIntelligenceFor(int type) {
    return std::min(validated(type), 5);
}

int Obstacle::requiredResilienceFor(int type) {
    return std::max(validated(type) - 5, 0);
}

bool Obstacle::isValidType(int type) {
    return type >= kMinType && type <= kMaxType;
}
