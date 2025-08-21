#include "Character.h"

Character::Character(int id, const std::string& name, const std::string& description, int room_id, const std::string& dialogue)
    : RoomObject(id, name, description, room_id), dialogue(dialogue) {
    // Constructor body is empty as initialization is done in the member initializer list.
}

std::string Character::getDialogue() const {
    return dialogue;
}
