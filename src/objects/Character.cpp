#include "Character.h"

Character::Character(const std::string& name, const std::string& description, const std::string& dialogue)
    : name(name), description(description), dialogue(dialogue) {
    // Constructor body is empty as initialization is done in the member initializer list.
}

std::string Character::getName() const {
    return name;
}

std::string Character::getDescription() const {
    return description;
}

std::string Character::getDialogue() const {
    return dialogue;
}
