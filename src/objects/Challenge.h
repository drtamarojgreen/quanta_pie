#ifndef CHALLENGE_H
#define CHALLENGE_H

#include <string>
#include <vector>
#include <functional>

// Forward declaration of Game class to avoid circular dependencies
// #include "../Game.h" // Removed to break circular dependency

/**
 * @struct CBTChoice
 * @brief Represents a single choice a player can make in response to a CBT challenge.
 */
struct CBTChoice {
    std::string description; // Text displayed for the choice
    std::function<void()> action; // Action to perform if this choice is selected (no Game& argument)
};

/**
 * @class Challenge
 * @brief Represents a Cognitive Behavioral Therapy challenge in the game.
 *
 * A challenge presents a situation or thought pattern that the player needs to address
 * using CBT principles. It offers a set of choices, each with a specific outcome.
 */
class Challenge {
public:
    Challenge(const std::string& thought, const std::vector<CBTChoice>& choices);

    std::string getThought() const;
    const std::vector<CBTChoice>& getChoices() const;

private:
    std::string thought_pattern; // The negative thought or situation presented
    std::vector<CBTChoice> cbt_choices; // Available CBT-based responses
};

#endif // CHALLENGE_H
