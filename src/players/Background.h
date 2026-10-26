#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <string>
#include <vector>

/**
 * @enum Background
 * @brief The starting background a player chooses for a run.
 *
 * Backgrounds change health, the dash and resistance to being shaken. None of them
 * changes Emotional Intelligence or Resilience: every obstacle still costs exactly
 * what its type demands.
 */
enum class Background {
    Balanced,
    Steadfast,
    Swift,
    Grounded
};

struct BackgroundTraits {
    Background background;
    const char* name;        // As written in config.txt and save files
    const char* title;       // As shown to the player
    const char* description;
    int max_health;
    int dash_cooldown;       // Turns before the dash can be used again
    bool shaken_immune;
};

const BackgroundTraits& traitsOf(Background background);
const std::vector<BackgroundTraits>& allBackgrounds();
bool backgroundFromName(const std::string& name, Background& background); // False if unknown

#endif // BACKGROUND_H
