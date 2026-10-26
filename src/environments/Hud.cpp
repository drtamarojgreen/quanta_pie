#include "Hud.h"
#include "Gauntlet.h"
#include <algorithm>
#include <set>

namespace {

const std::string kGap = "   ";
const size_t kTextWidth = 78;

std::string padRight(const std::string& text, size_t width) {
    return text.size() >= width ? text : text + std::string(width - text.size(), ' ');
}

std::string stateName(GameState state) {
    switch (state) {
        case GameState::Playing:          return "playing";
        case GameState::Looking:          return "looking";
        case GameState::Dashing:          return "dashing";
        case GameState::ChoosingSaveSlot: return "saving";
        case GameState::ChoosingLoadSlot: return "loading";
        case GameState::Paused:           return "paused";
        case GameState::Completed:        return "complete";
        case GameState::Defeated:         return "defeated";
    }
    return "";
}

std::vector<std::string> sideBySide(const std::vector<std::string>& left, const std::vector<std::string>& right) {
    size_t width = 0;
    for (const std::string& line : left) {
        width = std::max(width, line.size());
    }
    std::vector<std::string> lines;
    for (size_t i = 0; i < std::max(left.size(), right.size()); ++i) {
        std::string line = padRight(i < left.size() ? left[i] : "", width);
        if (i < right.size()) {
            line += kGap + right[i];
        }
        lines.push_back(line);
    }
    return lines;
}

// Breaks text into lines of at most width characters, at spaces where possible.
std::vector<std::string> wrap(const std::string& text, size_t width) {
    std::vector<std::string> lines;
    std::string line;
    size_t start = 0;
    while (start < text.size()) {
        size_t end = text.find(' ', start);
        std::string word = text.substr(start, end == std::string::npos ? std::string::npos : end - start);
        start = end == std::string::npos ? text.size() : end + 1;
        if (!line.empty() && line.size() + 1 + word.size() > width) {
            lines.push_back(line);
            line.clear();
        }
        line += (line.empty() ? "" : " ") + word;
    }
    if (!line.empty()) {
        lines.push_back(line);
    }
    return lines;
}

void append(std::vector<std::string>& lines, const std::vector<std::string>& more) {
    lines.insert(lines.end(), more.begin(), more.end());
}

std::string needs(const Obstacle& obstacle) {
    std::string text = std::to_string(obstacle.getRequiredEmotionalIntelligence()) + " EI";
    if (obstacle.getRequiredResilience() > 0) {
        text += " + " + std::to_string(obstacle.getRequiredResilience()) + " R";
    }
    return text;
}

std::string returnHint(const Gauntlet& game, Command command) {
    return "Press " + game.keyHint(command) + " again, or any action key, to return.";
}

std::string signedChange(int change) {
    if (change == 0) {
        return "";
    }
    return change > 0 ? " (+" + std::to_string(change) + ")" : " (" + std::to_string(change) + ")";
}

ConsoleColor healthColor(int health, int max_health) {
    if (health * 2 > max_health) {
        return ConsoleColor::Green;
    }
    return health * 4 > max_health ? ConsoleColor::Yellow : ConsoleColor::Red;
}

ConsoleColor toneColor(MessageTone tone) {
    switch (tone) {
        case MessageTone::Good:    return ConsoleColor::Green;
        case MessageTone::Bad:     return ConsoleColor::Red;
        case MessageTone::Tip:     return ConsoleColor::Cyan;
        case MessageTone::Neutral: break;
    }
    return ConsoleColor::Default;
}

std::vector<Hud::StyledLine> plain(const std::vector<std::string>& lines) {
    return std::vector<Hud::StyledLine>(lines.begin(), lines.end());
}

} // namespace

namespace Hud {

void StyledLine::paint(size_t from, size_t to, ConsoleColor color) {
    for (size_t i = from; i < std::min(to, colors.size()); ++i) {
        colors[i] = color;
    }
}

std::string meter(int have, int need, int width) {
    std::string bar = "[";
    for (int i = 0; i < width; ++i) {
        bar += i < std::min(have, need) ? '#' : i < need ? '-' : ' ';
    }
    return bar + "]";
}

std::string healthBar(int health, int max_health) {
    const int width = 10;
    int filled = max_health > 0 ? (health * width + max_health - 1) / max_health : 0; // Round up: any health shows
    return "[" + std::string(filled, '#') + std::string(width - filled, '-') + "]";
}

std::string formatTime(long long milliseconds) {
    long long seconds = milliseconds / 1000;
    long long hours = seconds / 3600;
    long long minutes = (seconds / 60) % 60;
    std::string ss = (seconds % 60 < 10 ? "0" : "") + std::to_string(seconds % 60);
    if (hours > 0) {
        return std::to_string(hours) + ":" + (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" + ss;
    }
    return std::to_string(minutes) + ":" + ss;
}

std::vector<std::string> renderRoom(const Gauntlet& game) {
    const RoomLayout& layout = game.getCurrentLayout();
    Room* room = game.getCurrentRoom();
    std::vector<std::string> grid(layout.height, std::string(layout.width, '.'));

    auto place = [&](const Position& at, char symbol) {
        if (layout.contains(at)) {
            grid[at.y][at.x] = symbol;
        }
    };
    for (const Hazard& hazard : layout.hazards) {
        place(hazard.position, ':');
    }
    for (RoomObject* object : room->getObjects()) {
        if (const Equipment* equipment = dynamic_cast<const Equipment*>(object)) {
            place(equipment->getPosition(), equipment->getKind() == EquipmentKind::EmotionalIntelligence ? 'E' : 'R');
        }
    }
    for (RoomObject* object : room->getObjects()) {
        const Obstacle* obstacle = dynamic_cast<const Obstacle*>(object);
        if (obstacle && !obstacle->isCleared()) {
            place(obstacle->getPosition(), obstacle->getSymbol());
        }
    }
    place(game.getPlayer().getPosition(), '@');

    auto border = [&](const Position& exit, bool open, char symbol) {
        std::string line = "+" + std::string(layout.width, '-') + "+";
        if (open) {
            line[exit.x + 1] = symbol;
        }
        return line;
    };
    std::vector<std::string> lines;
    lines.push_back(border(layout.door, room->getExit(Environment::kForward) != nullptr, '^'));
    for (const std::string& row : grid) {
        lines.push_back("|" + row + "|");
    }
    lines.push_back(border(layout.entry, room->getExit(Environment::kBack) != nullptr, 'v'));
    return lines;
}

std::vector<std::string> renderStatus(const Gauntlet& game) {
    const Player& player = game.getPlayer();
    const Obstacle* obstacle = game.getActiveObstacle();
    int need_ei = obstacle ? obstacle->getRequiredEmotionalIntelligence() : 0;
    int need_r = obstacle ? obstacle->getRequiredResilience() : 0;
    int ei = player.getEmotionalIntelligence();
    int r = player.getResilience();
    const std::vector<Obstacle*>& obstacles = game.getEnvironment().getObstacles(game.getCurrentRoomNumber());
    int cleared = 0;
    for (const Obstacle* each : obstacles) {
        cleared += each->isCleared() ? 1 : 0;
    }
    int shaken = player.getStatusTurns(StatusEffect::Shaken);

    return {
        game.getRoomTitle(game.getCurrentRoomNumber()) + " (" + std::to_string(game.getCurrentRoomNumber()) + "/" +
            std::to_string(game.getRoomCount()) + ")",
        "Obstacles cleared " + std::to_string(cleared) + "/" + std::to_string(obstacles.size()) +
            (obstacle ? ", next " + obstacle->getLabel() : ", door open"),
        "Health " + healthBar(player.getHealth(), player.getMaxHealth()) + " " + std::to_string(player.getHealth()) +
            "/" + std::to_string(player.getMaxHealth()) + signedChange(game.getLastHealthChange()),
        "EI " + meter(ei, need_ei) + " " + std::to_string(ei) + "/" + std::to_string(need_ei),
        "R  " + meter(r, need_r) + " " + std::to_string(r) + "/" + std::to_string(need_r),
        "Load " + std::to_string(player.getCarryWeight()) + "/" + std::to_string(player.getCarryCapacity()) +
            "  Score " + std::to_string(player.getScore()),
        "Turn " + std::to_string(game.getStats().getTurns()) + "  Time " +
            formatTime(game.getElapsedMilliseconds()) + "  Mode: " + stateName(game.getState()),
        game.getDashCooldown() == 0 ? "Dash: ready (" + game.keyHint(Command::Dash) + ")"
                                    : "Dash: " + std::to_string(game.getDashCooldown()) + " turns",
        shaken > 0 ? "Status: Shaken (" + std::to_string(shaken) + " turns)" : "Status: steady",
    };
}

std::vector<std::string> renderObstacles(const Gauntlet& game) {
    const Obstacle* active = game.getActiveObstacle();
    std::vector<std::string> lines = {"In this room:"};
    for (const Obstacle* obstacle : game.getEnvironment().getObstacles(game.getCurrentRoomNumber())) {
        std::string mark = obstacle->isCleared() ? "done" : obstacle == active ? "next" : "";
        lines.push_back("  " + padRight(mark, 5) + padRight(obstacle->getLabel(), 25) + needs(*obstacle));
    }
    return lines;
}

std::vector<std::string> renderMiniMap(const Gauntlet& game) {
    const int per_row = 10;
    const Environment& environment = game.getEnvironment();
    std::vector<std::string> lines = {"Progress (# cleared, @ you, ahead: each room's hardest obstacle)"};
    for (int first = 1; first <= game.getRoomCount(); first += per_row) {
        std::string row = padRight(std::to_string(first), 4);
        for (int number = first; number < first + per_row && number <= game.getRoomCount(); ++number) {
            row += number == game.getCurrentRoomNumber() ? '@'
                 : environment.isRoomCleared(number) ? '#'
                 : environment.getObstacles(number).back()->getSymbol();
        }
        lines.push_back(row);
    }
    return lines;
}

std::vector<std::string> renderInventory(const Gauntlet& game) {
    const Player& player = game.getPlayer();
    std::vector<std::string> lines = {
        "Inventory (load " + std::to_string(player.getCarryWeight()) + "/" + std::to_string(player.getCarryCapacity()) +
            ", " + std::to_string(player.getEmotionalIntelligence()) + " EI, " +
            std::to_string(player.getResilience()) + " R)",
    };
    for (const Tool* tool : player.getTools()) {
        const Equipment* equipment = dynamic_cast<const Equipment*>(tool);
        if (!equipment) {
            lines.push_back("  - " + tool->getName());
            continue;
        }
        bool ei = equipment->getKind() == EquipmentKind::EmotionalIntelligence;
        lines.push_back(std::string("  ") + (ei ? 'E' : 'R') + " " + equipment->getName() + " (+" +
                        std::to_string(equipment->getPoints()) + (ei ? " EI)" : " R)"));
    }
    if (player.getTools().empty()) {
        lines.push_back("  (empty)");
    }
    lines.push_back("");
    lines.push_back(returnHint(game, Command::ShowInventory));
    return lines;
}

std::vector<std::string> renderStats(const Gauntlet& game) {
    const PlayerStats& stats = game.getStats();
    const GameOptions& options = game.getOptions();
    std::vector<std::string> lines = {
        "Statistics",
        "  " + std::string(traitsOf(options.background).title) + (options.hardcore ? ", hardcore" : "") +
            ", time " + formatTime(game.getElapsedMilliseconds()),
        "  Turns " + std::to_string(stats.getTurns()) + ", steps " + std::to_string(stats.getSteps()) +
            ", waits " + std::to_string(stats.getWaits()) + ", dashes " + std::to_string(stats.getDashes()) +
            ", blocked moves " + std::to_string(stats.getBlockedMoves()),
        "  Furthest room " + std::to_string(stats.getFurthestRoom()) + ", times turned back " +
            std::to_string(stats.getBacktracks()),
        "  EI collected " + std::to_string(stats.getEmotionalIntelligenceCollected()) + ", spent " +
            std::to_string(stats.getEmotionalIntelligenceSpent()),
        "  R collected " + std::to_string(stats.getResilienceCollected()) + ", spent " +
            std::to_string(stats.getResilienceSpent()),
        "  Health lost " + std::to_string(stats.getDamageTaken()) + ", recovered " +
            std::to_string(stats.getHealingReceived()) + ", hazards " + std::to_string(stats.getHazardsHit()) +
            ", strikes " + std::to_string(stats.getStrikesTaken()) + ", collapses " + std::to_string(stats.getCollapses()),
        "  Obstacles cleared " + std::to_string(stats.getObstaclesCleared()) + ":",
    };
    for (int type = Obstacle::kMinType; type <= Obstacle::kMaxType; ++type) {
        std::string name = game.getEnvironment().getObstacleName(type);
        if (name.empty()) {
            continue; // This world's data does not use the type
        }
        lines.push_back("    " + padRight(std::to_string(type), 3) +
                        padRight(std::string(1, game.getEnvironment().getObstacleSymbol(type)) + " " + name, 25) +
                        std::to_string(stats.getObstaclesCleared(type)));
    }
    lines.push_back("Achievements");
    for (int i = 0; i < static_cast<int>(Achievement::Count); ++i) {
        Achievement achievement = static_cast<Achievement>(i);
        lines.push_back(std::string("  [") + (stats.hasAchievement(achievement) ? 'x' : ' ') + "] " +
                        PlayerStats::achievementName(achievement) + " - " +
                        PlayerStats::achievementDescription(achievement));
    }
    lines.push_back("");
    lines.push_back(returnHint(game, Command::ShowStats));
    return lines;
}

std::vector<std::string> renderBestiary(const Gauntlet& game) {
    const Environment& environment = game.getEnvironment();
    std::vector<std::string> lines = {"Bestiary (obstacles fill in as you meet them)"};
    for (int type = Obstacle::kMinType; type <= Obstacle::kMaxType; ++type) {
        std::string name = environment.getObstacleName(type);
        if (name.empty()) {
            continue;
        }
        if (!game.getStats().hasSeen(type)) {
            lines.push_back("  " + padRight(std::to_string(type), 3) + "? ???  (not met yet)");
            continue;
        }
        int ei = Obstacle::requiredEmotionalIntelligenceFor(type);
        int r = Obstacle::requiredResilienceFor(type);
        // Look up a description from any obstacle of this type.
        std::string description;
        for (const auto& obstacle : environment.getAllObstacles()) {
            if (obstacle->getType() == type) {
                description = obstacle->getDescription();
                break;
            }
        }
        lines.push_back("  " + padRight(std::to_string(type), 3) + environment.getObstacleSymbol(type) + " " + name +
                        "  needs " + std::to_string(ei) + " EI" + (r > 0 ? " + " + std::to_string(r) + " R" : "") +
                        ", strikes for " + std::to_string(Gauntlet::kStrikePerType * type) + ", cleared " +
                        std::to_string(game.getStats().getObstaclesCleared(type)));
        lines.push_back("       " + description);
    }
    lines.push_back("");
    lines.push_back(returnHint(game, Command::ShowBestiary));
    return lines;
}

std::vector<std::string> renderHelp(const Gauntlet& game) {
    const Command commands[] = {
        Command::MoveNorthWest, Command::MoveNorth, Command::MoveNorthEast, Command::MoveWest,
        Command::MoveEast, Command::MoveSouthWest, Command::MoveSouth, Command::MoveSouthEast,
        Command::Wait, Command::Look, Command::Dash, Command::Clear, Command::Pause, Command::ShowInventory,
        Command::ShowStats, Command::ShowBestiary, Command::ShowHelp, Command::Save, Command::Load,
        Command::NewGame, Command::Quit,
    };
    std::vector<std::string> lines = {
        "Controls",
        "  Walk onto E (Emotional Intelligence) and R (Resilience) tiles to collect them.",
        "  Clear a room's obstacles in the order listed, standing next to each one;",
        "  clearing spends what it needs and brings new E and R for the next.",
        "  Each obstacle type has its own symbol. The door opens when all are cleared.",
        "  Every move costs 1 health (2 while shaken), and ':' hazards cost more.",
        "  Clearing an obstacle restores 10 + 3 per type. Attempting one unprepared",
        "  makes it strike back and leaves you shaken; wait to breathe it off.",
        "  At 0 health you collapse back to the room's entry (hardcore: the run ends).",
        "",
    };
    for (Command command : commands) {
        std::string keys;
        for (char key : game.getKeyBindings().keysFor(command)) {
            keys += keys.empty() ? std::string(1, key) : std::string(" ") + key;
        }
        lines.push_back("  " + padRight(keys.empty() ? "-" : keys, 6) + KeyBindings::commandName(command));
    }
    lines.push_back("  Arrow keys and Home/End/PgUp/PgDn also move.");
    lines.push_back("");
    lines.push_back(returnHint(game, Command::ShowHelp));
    return lines;
}

std::vector<StyledLine> renderStyledScreen(const Gauntlet& game) {
    switch (game.getView()) {
        case GameView::Inventory: return plain(renderInventory(game));
        case GameView::Stats:     return plain(renderStats(game));
        case GameView::Bestiary:  return plain(renderBestiary(game));
        case GameView::Help:      return plain(renderHelp(game));
        case GameView::Room:      break;
    }

    std::vector<std::string> room = renderRoom(game);
    std::vector<std::string> panel = renderStatus(game);
    const size_t health_line = 2;
    const size_t status_line = 8;
    panel.push_back("");
    append(panel, renderObstacles(game));
    std::vector<StyledLine> lines = plain(sideBySide(room, panel));
    const size_t room_width = room.front().size();
    const size_t panel_column = room_width + kGap.size();

    // Room grid: each kind of thing in its own colour
    std::set<char> obstacle_symbols;
    for (int type = Obstacle::kMinType; type <= Obstacle::kMaxType; ++type) {
        if (char symbol = game.getEnvironment().getObstacleSymbol(type)) {
            obstacle_symbols.insert(symbol);
        }
    }
    for (size_t row = 1; row + 1 < room.size(); ++row) {
        for (size_t column = 1; column + 1 < room_width; ++column) {
            char tile = lines[row].text[column];
            ConsoleColor color = tile == '@' ? ConsoleColor::White
                               : tile == 'E' ? ConsoleColor::Cyan
                               : tile == 'R' ? ConsoleColor::Green
                               : tile == ':' ? ConsoleColor::Magenta
                               : obstacle_symbols.count(tile) ? ConsoleColor::Red
                               : ConsoleColor::Default;
            lines[row].paint(column, column + 1, color);
        }
    }
    const Player& player = game.getPlayer();
    lines[health_line].paint(panel_column, std::string::npos, healthColor(player.getHealth(), player.getMaxHealth()));
    if (player.hasStatus(StatusEffect::Shaken)) {
        lines[status_line].paint(panel_column, std::string::npos, ConsoleColor::Yellow);
    }

    auto add = [&lines](const std::string& text, ConsoleColor color = ConsoleColor::Default) {
        lines.emplace_back(text);
        lines.back().paint(0, text.size(), color);
    };
    add("");
    for (const std::string& line : wrap(game.getCurrentRoom()->getDescription(), kTextWidth)) {
        add(line);
    }
    add("");
    std::vector<std::string> map = renderMiniMap(game);
    add(map.front());
    for (size_t i = 1; i < map.size(); ++i) {
        lines.emplace_back(map[i]);
        for (size_t column = 4; column < map[i].size(); ++column) {
            char room_mark = map[i][column];
            lines.back().paint(column, column + 1, room_mark == '#' ? ConsoleColor::Green
                                                 : room_mark == '@' ? ConsoleColor::White : ConsoleColor::Red);
        }
    }
    add("");
    add("Objective: " + game.getObjective(), game.getState() == GameState::Defeated ? ConsoleColor::Red
                                           : game.getState() == GameState::Completed ? ConsoleColor::Green
                                                                                     : ConsoleColor::Default);
    add("");
    const auto& messages = game.getMessageLog().getMessages();
    const auto& tones = game.getMessageLog().getTones();
    for (size_t i = 0; i < messages.size(); ++i) {
        add("> " + messages[i], toneColor(tones[i]));
    }
    add("");
    add("Press " + game.keyHint(Command::ShowHelp) + " for help.");
    return lines;
}

std::vector<std::string> renderScreen(const Gauntlet& game) {
    std::vector<std::string> lines;
    for (const StyledLine& line : renderStyledScreen(game)) {
        lines.push_back(line.text);
    }
    return lines;
}

} // namespace Hud
