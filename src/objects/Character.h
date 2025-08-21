#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>

class Character {
public:
    Character(const std::string& name, const std::string& description, const std::string& dialogue);

    std::string getName() const;
    std::string getDescription() const;
    std::string getDialogue() const;

private:
    std::string name;
    std::string description;
    std::string dialogue;
};

#endif // CHARACTER_H
