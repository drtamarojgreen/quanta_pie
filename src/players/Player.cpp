#include "Player.h"
#include "../Room.h"
#include "../objects/Obstacle.h"
#include <algorithm>

Player::Player(int id, const std::string& name, const std::string& joinDate, Room* startingRoom)
    : id(id), name(name), joinDate(joinDate), currentRoom(startingRoom), score(0), obstaclesCleared(0),
      carryCapacity(kDefaultCarryCapacity), maxHealth(kDefaultMaxHealth), health(kDefaultMaxHealth),
      carryWeight(0), emotionalIntelligence(0), resilience(0) {}

Player::Player(Room* startingRoom)
    : Player(0, "Player", "N/A", startingRoom) {}

bool Player::move(const std::string& direction) {
    if (currentRoom && !getBlockingObstacle(direction)) {
        Room* nextRoom = currentRoom->getExit(direction);
        if (nextRoom) {
            setCurrentRoom(nextRoom);
            return true;
        }
    }
    return false;
}

void Player::setCurrentRoom(Room* room) {
    currentRoom = room;
}

Room* Player::getCurrentRoom() const {
    return currentRoom;
}

int Player::getID() const {
    return id;
}

std::string Player::getName() const {
    return name;
}

std::string Player::getJoinDate() const {
    return joinDate;
}

std::string Player::getRepresentation() const {
    return "@";
}

int Player::getScore() const {
    return score;
}

void Player::incrementScore(int amount) {
    score += amount;
}

Position Player::getPosition() const {
    return position;
}

void Player::setPosition(const Position& position) {
    this->position = position;
}

bool Player::takeTool(Tool* tool) {
    if (!tool || carryWeight + tool->getWeight() > carryCapacity) {
        return false;
    }
    tools.push_back(tool);
    if (currentRoom) {
        currentRoom->removeObject(tool);
    }
    recalculateInventory();
    return true;
}

const std::vector<Tool*>& Player::getTools() const {
    return tools;
}

void Player::dropTool(Tool* tool) {
    if (tool && currentRoom) {
        // Remove the tool from the player's inventory
        tools.erase(std::remove(tools.begin(), tools.end(), tool), tools.end());
        // Add the tool to the current room, where the player is standing
        tool->setPosition(position);
        currentRoom->addObject(tool);
        recalculateInventory();
    }
}

int Player::getCarryWeight() const {
    return carryWeight;
}

int Player::getCarryCapacity() const {
    return carryCapacity;
}

void Player::setCarryCapacity(int capacity) {
    carryCapacity = capacity;
}

void Player::recalculateInventory() {
    carryWeight = 0;
    emotionalIntelligence = 0;
    resilience = 0;
    for (const Tool* tool : tools) {
        carryWeight += tool->getWeight();
        const Equipment* equipment = dynamic_cast<const Equipment*>(tool);
        if (!equipment) {
            continue;
        }
        if (equipment->getKind() == EquipmentKind::EmotionalIntelligence) {
            emotionalIntelligence += equipment->getPoints();
        } else {
            resilience += equipment->getPoints();
        }
    }
}

int Player::getEmotionalIntelligence() const {
    return emotionalIntelligence;
}

int Player::getResilience() const {
    return resilience;
}

int Player::pickUpEquipment() {
    if (!currentRoom) {
        return 0;
    }
    // Copy first: takeTool removes objects from the room we are iterating.
    std::vector<RoomObject*> objects = currentRoom->getObjects();
    int taken = 0;
    for (RoomObject* object : objects) {
        Equipment* equipment = dynamic_cast<Equipment*>(object);
        if (equipment && takeTool(equipment)) {
            taken++;
        }
    }
    return taken;
}

bool Player::canClear(const Obstacle* obstacle) const {
    if (!obstacle || obstacle->isCleared()) {
        return false;
    }
    if (position.isValid() && obstacle->getPosition().isValid() &&
        position.distanceTo(obstacle->getPosition()) > 1) {
        return false;
    }
    return obstacle->meetsRequirements(getEmotionalIntelligence(), getResilience());
}

void Player::spendEquipment(EquipmentKind kind, int points) {
    for (auto it = tools.begin(); it != tools.end() && points > 0;) {
        Equipment* equipment = dynamic_cast<Equipment*>(*it);
        if (equipment && equipment->getKind() == kind) {
            points -= equipment->getPoints();
            equipment->consume();
            it = tools.erase(it);
        } else {
            ++it;
        }
    }
}

bool Player::clearObstacle(Obstacle* obstacle) {
    if (!canClear(obstacle)) {
        return false;
    }
    spendEquipment(EquipmentKind::EmotionalIntelligence, obstacle->getRequiredEmotionalIntelligence());
    spendEquipment(EquipmentKind::Resilience, obstacle->getRequiredResilience());
    recalculateInventory();
    obstacle->clear();
    if (currentRoom) {
        currentRoom->removeObject(obstacle);
    }
    obstaclesCleared++;
    incrementScore(obstacle->getType());
    return true;
}

Obstacle* Player::getBlockingObstacle(const std::string& direction) const {
    if (!currentRoom) {
        return nullptr;
    }
    for (RoomObject* object : currentRoom->getObjects()) {
        Obstacle* obstacle = dynamic_cast<Obstacle*>(object);
        if (obstacle && obstacle->blocks(direction)) {
            return obstacle;
        }
    }
    return nullptr;
}

int Player::getObstaclesCleared() const {
    return obstaclesCleared;
}

int Player::getHealth() const {
    return health;
}

int Player::getMaxHealth() const {
    return maxHealth;
}

void Player::setMaxHealth(int max_health) {
    maxHealth = max_health;
    health = max_health;
}

void Player::restoreHealth() {
    health = maxHealth;
}

int Player::damage(int amount) {
    int lost = std::min(std::max(amount, 0), health);
    health -= lost;
    return lost;
}

int Player::heal(int amount) {
    int gained = std::min(std::max(amount, 0), maxHealth - health);
    health += gained;
    return gained;
}

bool Player::isDefeated() const {
    return health <= 0;
}

void Player::addStatus(StatusEffect effect, int turns) {
    if (turns > 0) {
        statuses[effect] = std::max(statuses[effect], turns);
    }
}

void Player::removeStatus(StatusEffect effect) {
    statuses.erase(effect);
}

bool Player::hasStatus(StatusEffect effect) const {
    return statuses.count(effect) > 0;
}

int Player::getStatusTurns(StatusEffect effect) const {
    auto found = statuses.find(effect);
    return found == statuses.end() ? 0 : found->second;
}

void Player::tickStatuses() {
    for (auto it = statuses.begin(); it != statuses.end();) {
        if (--it->second <= 0) {
            it = statuses.erase(it);
        } else {
            ++it;
        }
    }
}
