#include "Background.h"

const std::vector<BackgroundTraits>& allBackgrounds() {
    static const std::vector<BackgroundTraits> backgrounds = {
        {Background::Balanced, "balanced", "Balanced", "100 health; dash every 5 turns.", 100, 5, false},
        {Background::Steadfast, "steadfast", "Steadfast", "130 health; dash every 8 turns.", 130, 8, false},
        {Background::Swift, "swift", "Swift", "80 health; dash every 2 turns.", 80, 2, false},
        {Background::Grounded, "grounded", "Grounded", "100 health; dash every 5 turns; never shaken.", 100, 5, true},
    };
    return backgrounds;
}

const BackgroundTraits& traitsOf(Background background) {
    for (const BackgroundTraits& traits : allBackgrounds()) {
        if (traits.background == background) {
            return traits;
        }
    }
    return allBackgrounds().front();
}

bool backgroundFromName(const std::string& name, Background& background) {
    for (const BackgroundTraits& traits : allBackgrounds()) {
        if (name == traits.name) {
            background = traits.background;
            return true;
        }
    }
    return false;
}
