#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>

/**
 * @class Character
 * @brief Represents a non-player character (NPC) in the game.
 *
 * This class stores all information related to a character, including their
 * name, a detailed description, and a line of dialogue they can speak.
 */
class Character {
public:
    /**
     * @brief Constructs a new Character object.
     * @param name The name of the character.
     * @param description A description of the character's appearance and demeanor.
     * @param dialogue A line of dialogue the character can say.
     */
    Character(const std::string& name, const std::string& description, const std::string& dialogue);

    /**
     * @brief Gets the character's name.
     * @return The name of the character.
     */
    std::string getName() const;

    /**
     * @brief Gets the character's description.
     * @return The description of the character.
     */
    std::string getDescription() const;

    /**
     * @brief Gets the character's dialogue.
     * @return The character's line of dialogue.
     */
    std::string getDialogue() const;

private:
    std::string name;
    std::string description;
    std::string dialogue;
};

#endif // CHARACTER_H
