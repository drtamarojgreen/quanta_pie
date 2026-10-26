#include "Game.h"
#include "platform/Console.h"
#ifdef _WIN32
#include "platform/WindowsConsole.h" // Include the Windows-specific console implementation
#else
#include "platform/PosixConsole.h"   // Include the Linux/POSIX console implementation
#endif
#include <string>
#include <vector>

namespace {

// Windows' _getch() (and PosixConsole) report arrow and navigation keys as a prefix
// of 224 (or 0) followed by a scan code.
const int kExtendedKeyPrefix = 224;
const int kFunctionKeyPrefix = 0;
const int kEnter = '\r';

std::unique_ptr<Console> makePlatformConsole() {
#ifdef _WIN32
    return std::make_unique<WindowsConsole>();
#else
    return std::make_unique<PosixConsole>();
#endif
}

Command commandForScanCode(int scan_code) {
    switch (scan_code) {
        case 72: return Command::MoveNorth;     // Up arrow
        case 80: return Command::MoveSouth;     // Down arrow
        case 75: return Command::MoveWest;      // Left arrow
        case 77: return Command::MoveEast;      // Right arrow
        case 71: return Command::MoveNorthWest; // Home
        case 73: return Command::MoveNorthEast; // Page Up
        case 79: return Command::MoveSouthWest; // End
        case 81: return Command::MoveSouthEast; // Page Down
        default: return Command::None;
    }
}

const char* eventName(GameEventType type) {
    switch (type) {
        case GameEventType::RoomEntered:         return "RoomEntered";
        case GameEventType::PickedUp:            return "PickedUp";
        case GameEventType::InventoryFull:       return "InventoryFull";
        case GameEventType::Blocked:             return "Blocked";
        case GameEventType::Cleared:             return "Cleared";
        case GameEventType::CannotClear:         return "CannotClear";
        case GameEventType::EquipmentSpawned:    return "EquipmentSpawned";
        case GameEventType::Struck:              return "Struck";
        case GameEventType::HazardHit:           return "HazardHit";
        case GameEventType::Healed:              return "Healed";
        case GameEventType::StatusChanged:       return "StatusChanged";
        case GameEventType::Dashed:              return "Dashed";
        case GameEventType::Collapsed:           return "Collapsed";
        case GameEventType::Defeated:            return "Defeated";
        case GameEventType::Looked:              return "Looked";
        case GameEventType::Waited:              return "Waited";
        case GameEventType::Paused:              return "Paused";
        case GameEventType::Resumed:             return "Resumed";
        case GameEventType::Prompt:              return "Prompt";
        case GameEventType::Ignored:             return "Ignored";
        case GameEventType::Saved:               return "Saved";
        case GameEventType::SaveFailed:          return "SaveFailed";
        case GameEventType::Loaded:              return "Loaded";
        case GameEventType::LoadFailed:          return "LoadFailed";
        case GameEventType::AchievementUnlocked: return "AchievementUnlocked";
        case GameEventType::Tip:                 return "Tip";
        case GameEventType::Completed:           return "Completed";
    }
    return "Event";
}

bool sameLine(const Hud::StyledLine& a, const Hud::StyledLine& b) {
    return a.text == b.text && a.colors == b.colors;
}

GameConfig withDataDirectory(GameConfig config, const std::string& data_directory) {
    config.data_directory = data_directory;
    return config;
}

} // namespace

Game::Game() : Game(GameConfig::loadFile(GameConfig::kDefaultFile), makePlatformConsole()) {}

Game::Game(const std::string& data_directory)
    : Game(withDataDirectory(GameConfig::loadFile(GameConfig::kDefaultFile), data_directory), makePlatformConsole()) {}

Game::Game(const GameConfig& config, std::unique_ptr<Console> console, std::ostream& output)
    : config(config), console(std::move(console)), output(output),
      gauntlet(config.data_directory, config.options), shake_next_frame(false) {
    config.applyBindings(gauntlet.getKeyBindings());
    if (!config.log_file.empty()) {
        log.open(config.log_file, std::ios::app);
    }
}

Game::~Game() = default; // Explicitly defaulted in .cpp file

void Game::start() {
    if (runWelcomeMenu()) {
        gameLoop();
    }
    if (config.color) {
        console->setColor(ConsoleColor::Default);
    }
    output << std::endl << "Thank you for playing Quanta_Pie!" << std::endl;
    if (log.is_open() && config.profile) {
        log << "Profile:" << std::endl;
        for (const std::string& line : profiler.report()) {
            log << "  " << line << std::endl;
        }
    }
}

const Gauntlet& Game::getGauntlet() const {
    return gauntlet;
}

const Profiler& Game::getProfiler() const {
    return profiler;
}

bool Game::runWelcomeMenu() {
    GameOptions options = gauntlet.getOptions();
    bool choosing_slot = false;
    while (true) {
        console->clear();
        output << "Welcome to Quanta_Pie!" << std::endl << std::endl;
        output << "Work your way through " << gauntlet.getRoomCount() << " rooms, each harder than the last. Every room is" << std::endl;
        output << "guarded by a row of emotional obstacles, the enemies here, that take Emotional Intelligence" << std::endl;
        output << "(EI) and, later, Resilience (R) to clear. Each kind has its own symbol on the map." << std::endl;
        output << "Walk onto the E and R tiles to collect them, stand next to the next obstacle and press "
               << gauntlet.keyHint(Command::Clear) << "." << std::endl;
        output << "Every move costs health; clearing an obstacle restores some. Facing one unprepared hurts." << std::endl;
        output << std::endl << "Background (press 1-" << allBackgrounds().size() << "):" << std::endl;
        int number = 1;
        for (const BackgroundTraits& traits : allBackgrounds()) {
            output << "  [" << (traits.background == options.background ? 'x' : ' ') << "] " << number++ << " "
                   << traits.title << ": " << traits.description << std::endl;
        }
        output << "Mode (press H): " << (options.hardcore ? "hardcore, where collapsing ends the run and deletes its save"
                                                          : "normal, where collapsing sends you back to the room's entry")
               << std::endl;
        output << "Press L and a slot number (1-" << Gauntlet::kSaveSlots << ") to continue a saved game." << std::endl;
        output << std::endl << (choosing_slot ? "Load which slot?" : "Press Enter to begin...") << std::endl;

        int key = console->getChar();
        if (key < 0 || g_signal_received) {
            return false;
        }
        if (choosing_slot) {
            choosing_slot = false;
            int slot = key - '0';
            GameEvent event;
            if (slot >= 1 && slot <= Gauntlet::kSaveSlots && gauntlet.loadFromSlot(slot)) {
                while (gauntlet.pollEvent(event)) {
                    logEvent(event);
                }
                return true;
            }
            while (gauntlet.pollEvent(event)) {
                logEvent(event);
            }
            continue;
        }
        int choice = key - '0';
        if (choice >= 1 && choice <= static_cast<int>(allBackgrounds().size())) {
            options.background = allBackgrounds()[choice - 1].background;
        } else if (key == 'h' || key == 'H') {
            options.hardcore = !options.hardcore;
        } else if (key == 'l' || key == 'L') {
            choosing_slot = true;
        } else if (key == kEnter) {
            gauntlet.newGame(options);
            return true;
        }
    }
}

void Game::writeStyled(const Hud::StyledLine& line) {
    if (!config.color) {
        output << line.text;
        return;
    }
    size_t start = 0;
    while (start < line.text.size()) {
        size_t end = start;
        while (end < line.text.size() && line.colors[end] == line.colors[start]) {
            end++;
        }
        console->setColor(line.colors[start]);
        output << line.text.substr(start, end - start);
        start = end;
    }
    console->setColor(ConsoleColor::Default);
}

void Game::displayGameScreen() {
    Profiler::Scope timing(profiler, "render");
    std::vector<Hud::StyledLine> next = Hud::renderStyledScreen(gauntlet);
    next.emplace_back("");
    next.emplace_back("> ");
    if (shake_next_frame) {
        // A jolt for a hit: this frame is drawn one column to the right.
        for (Hud::StyledLine& line : next) {
            line.text.insert(line.text.begin(), ' ');
            line.colors.insert(line.colors.begin(), ConsoleColor::Default);
        }
        shake_next_frame = false;
    }

    bool full = frame.empty();
    if (full) {
        console->clear();
    }
    size_t rows = std::max(next.size(), frame.size());
    Hud::StyledLine blank;
    for (size_t row = 0; row < rows; ++row) {
        const Hud::StyledLine& line = row < next.size() ? next[row] : blank;
        if (!full && row < frame.size() && sameLine(frame[row], line)) {
            continue; // Unchanged: leave it on screen
        }
        console->setCursorPosition(0, static_cast<int>(row));
        writeStyled(line);
        size_t old_width = row < frame.size() ? frame[row].text.size() : 0;
        if (old_width > line.text.size()) {
            output << std::string(old_width - line.text.size(), ' '); // Erase what the old line left behind
        }
    }
    frame = next;
    console->setCursorPosition(static_cast<int>(frame.back().text.size()), static_cast<int>(frame.size()) - 1);
    output << std::flush;
}

void Game::logEvent(const GameEvent& event) {
    if (log.is_open()) {
        log << "[turn " << gauntlet.getStats().getTurns() << "] " << eventName(event.type) << ": "
            << event.message << std::endl;
    }
}

void Game::processKey(int key) {
    {
        Profiler::Scope timing(profiler, "input");
        if (key == kExtendedKeyPrefix || key == kFunctionKeyPrefix) {
            gauntlet.execute(commandForScanCode(console->getChar()));
        } else if (key > 0 && key < 128) {
            gauntlet.handleKey(static_cast<char>(key));
        }
    }
    GameEvent event;
    while (gauntlet.pollEvent(event)) {
        logEvent(event);
        if (event.type == GameEventType::Struck || event.type == GameEventType::HazardHit ||
            event.type == GameEventType::Collapsed || event.type == GameEventType::Defeated) {
            shake_next_frame = true;
        }
    }
}

void Game::gameLoop() {
    frame.clear(); // The welcome screen is up: draw the first frame in full
    while (!gauntlet.isQuitRequested() && !g_signal_received) {
        displayGameScreen(); // Refresh screen at the start of every turn
        int key = console->getChar();
        if (key < 0) {
            break; // End of input
        }
        processKey(key);
    }
}
