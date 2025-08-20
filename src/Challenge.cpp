#include "Challenge.h"

Challenge::Challenge(const std::string& thought, const std::vector<CBTChoice>& choices)
    : thought_pattern(thought), cbt_choices(choices) {}

std::string Challenge::getThought() const {
    return thought_pattern;
}

const std::vector<CBTChoice>& Challenge::getChoices() const {
    return cbt_choices;
}
