#ifndef HUD_H
#define HUD_H

#include <string>
#include <vector>

#include "../platform/Console.h"

class Gauntlet;

/**
 * @namespace Hud
 * @brief Text rendering for a Gauntlet. Every function returns lines without trailing newlines,
 *        so any console (or a test) can display them.
 */
namespace Hud {

    /**
     * @struct StyledLine
     * @brief A line of text with a colour for each character.
     */
    struct StyledLine {
        std::string text;
        std::vector<ConsoleColor> colors; // Same length as text

        explicit StyledLine(const std::string& text = "")
            : text(text), colors(text.size(), ConsoleColor::Default) {}
        void paint(size_t from, size_t to, ConsoleColor color); // Colours [from, to), clipped to the text
    };

    // The room grid: @ you, E/R equipment, : hazards, ^ north exit, v south exit, and each
    // obstacle drawn with its type's own symbol
    std::vector<std::string> renderRoom(const Gauntlet& game);

    // Room, progress, health (with the last change), EI/R meters against what the next obstacle
    // needs, load, score, turn, time, mode, dash and status
    std::vector<std::string> renderStatus(const Gauntlet& game);

    // The room's obstacles in clearing order, with symbol, name, needs and whether each is done
    std::vector<std::string> renderObstacles(const Gauntlet& game);

    // One row of ten rooms per line: @ current, # cleared, otherwise the symbol of the room's
    // hardest obstacle
    std::vector<std::string> renderMiniMap(const Gauntlet& game);

    std::vector<std::string> renderInventory(const Gauntlet& game);
    std::vector<std::string> renderStats(const Gauntlet& game);
    std::vector<std::string> renderBestiary(const Gauntlet& game); // Obstacle types met so far
    std::vector<std::string> renderHelp(const Gauntlet& game);

    // The full frame for the game's current view, with colours
    std::vector<StyledLine> renderStyledScreen(const Gauntlet& game);
    // The same frame as plain text
    std::vector<std::string> renderScreen(const Gauntlet& game);

    // "[##-  ]": # held, - still missing, blank not needed
    std::string meter(int have, int need, int width = 5);
    // "[#######---]": health out of the maximum, in ten steps
    std::string healthBar(int health, int max_health);
    // "m:ss", or "h:mm:ss" from an hour
    std::string formatTime(long long milliseconds);

} // namespace Hud

#endif // HUD_H
