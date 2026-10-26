#include "TestRunner.h"
#include "TestData.h"
#include "../src/Game.h"
#include "../src/platform/Console.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

// A console that replays scripted key codes, then reports end of input (-1).
// It records what the game asked of it.
class ScriptedConsole : public Console {
public:
    explicit ScriptedConsole(std::vector<int> keys) : keys(std::move(keys)) {}

    void clear() override { clears++; }
    void setCursorPosition(int, int) override { cursor_moves++; }
    int getChar() override { return next < keys.size() ? keys[next++] : -1; }
    int getConsoleWidth() const override { return 120; }
    void setColor(ConsoleColor color) override { colors.push_back(color); }

    int clears = 0;
    int cursor_moves = 0;
    std::vector<ConsoleColor> colors;

private:
    std::vector<int> keys;
    size_t next = 0;
};

const int kEnter = '\r';
const int kArrowPrefix = 224;    // Windows' _getch() prefix for arrow keys
const int kFunctionPrefix = 0;   // The prefix some keypads send instead
const int kUp = 72;
const int kLeft = 75;

std::vector<int> keys(const std::string& text) {
    return std::vector<int>(text.begin(), text.end());
}

std::vector<int> operator+(std::vector<int> a, const std::vector<int>& b) {
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

GameConfig plainConfig() {
    GameConfig config;
    config.data_directory = kSqlDir;
    config.color = false;
    return config;
}

} // namespace

// BDD-style test: the console frontend waits for Enter, then turns arrow keys into moves
bool testGame_ArrowKeysAndQuit() {
    // GIVEN a game whose console will send: a stray key, Enter, Up, keypad Left, then Q
    std::ostringstream output;
    auto console = std::make_unique<ScriptedConsole>(
        std::vector<int>{'w', kEnter, kArrowPrefix, kUp, kFunctionPrefix, kLeft, 'Q'});
    Game game(plainConfig(), std::move(console), output);

    // WHEN it runs
    game.start();

    // THEN the key pressed before Enter was ignored, and the arrows moved the player
    const Gauntlet& gauntlet = game.getGauntlet();
    ASSERT_TRUE(gauntlet.getPlayer().getPosition() == Position(5, 5));
    ASSERT_TRUE(gauntlet.isQuitRequested());
    ASSERT_EQ(gauntlet.getStats().getSteps(), 2);
    ASSERT_TRUE(output.str().find("Welcome to Quanta_Pie!") != std::string::npos);
    ASSERT_TRUE(output.str().find("Room 1: Hall of Echoes I (1/100)") != std::string::npos);
    ASSERT_TRUE(output.str().find("Thank you for playing Quanta_Pie!") != std::string::npos);
    return true;
}

// BDD-style test: the welcome menu picks a background and the mode before the run starts
bool testGame_WelcomeMenuChoosesBackgroundAndMode() {
    std::ostringstream output;
    auto console = std::make_unique<ScriptedConsole>(std::vector<int>{'3', 'h', kEnter, 'Q'});
    Game game(plainConfig(), std::move(console), output);
    game.start();

    const Gauntlet& gauntlet = game.getGauntlet();
    ASSERT_TRUE(gauntlet.getOptions().background == Background::Swift);
    ASSERT_TRUE(gauntlet.getOptions().hardcore);
    ASSERT_EQ(gauntlet.getPlayer().getMaxHealth(), 80);
    ASSERT_TRUE(output.str().find("[x] 3 Swift: 80 health; dash every 2 turns.") != std::string::npos);
    ASSERT_TRUE(output.str().find("Mode (press H): hardcore") != std::string::npos);
    return true;
}

// BDD-style test: the welcome menu can continue a saved game
bool testGame_WelcomeMenuLoadsASlot() {
    std::filesystem::path saves = std::filesystem::temp_directory_path() / "quantapie_saves_welcome";
    std::filesystem::remove_all(saves);
    std::filesystem::create_directories(saves);
    GameOptions options;
    options.save_directory = saves.string();
    Gauntlet earlier(kSqlDir, options);
    earlier.execute(Command::MoveNorthWest);
    ASSERT_TRUE(earlier.saveToSlot(2));

    GameConfig config = plainConfig();
    config.options.save_directory = saves.string();
    std::ostringstream output;
    Game game(config, std::make_unique<ScriptedConsole>(std::vector<int>{'L', '2', 'Q'}), output);
    game.start();
    ASSERT_TRUE(game.getGauntlet().getPlayer().getPosition() == Position(5, 5));
    ASSERT_EQ(game.getGauntlet().getCurrentSlot(), 2);
    std::filesystem::remove_all(saves);
    return true;
}

// BDD-style test: the first room can be cleared through the console, and input ending stops the game
bool testGame_ClearsFirstRoomAndStopsAtEndOfInput() {
    std::ostringstream output;
    // Enter; q x4 walks to the EI spawn (2,2), crossing a hazard; e d d walks beside the obstacle;
    // f clears; e w leaves
    auto console = std::make_unique<ScriptedConsole>(std::vector<int>{kEnter} + keys("qqqqeddfew"));
    Game game(plainConfig(), std::move(console), output);
    game.start();

    const Gauntlet& gauntlet = game.getGauntlet();
    ASSERT_EQ(gauntlet.getCurrentRoomNumber(), 2);
    ASSERT_TRUE(gauntlet.getEnvironment().isRoomCleared(1));
    ASSERT_EQ(gauntlet.getStats().getHazardsHit(), 1);
    ASSERT_TRUE(!gauntlet.isQuitRequested());
    ASSERT_TRUE(output.str().find("Thank you for playing Quanta_Pie!") != std::string::npos);
    return true;
}

// Test case: after the first frame, only the lines that changed are redrawn
bool testGame_RedrawsOnlyChangedLines() {
    std::ostringstream output;
    auto console = std::make_unique<ScriptedConsole>(std::vector<int>{kEnter, 's', 's'});
    ScriptedConsole* script = console.get();
    Game game(plainConfig(), std::move(console), output);
    game.start();

    // One clear for the welcome screen and one for the first frame, none after
    ASSERT_EQ(script->clears, 2);
    // The first frame positions every line; the waits change only a handful
    size_t lines = Hud::renderScreen(game.getGauntlet()).size() + 2;
    ASSERT_TRUE(script->cursor_moves > static_cast<int>(lines));
    ASSERT_TRUE(script->cursor_moves < static_cast<int>(lines) + 20);
    return true;
}

// Test case: colours follow the config, and key bindings from it apply
bool testGame_ColorsAndConfigBindings() {
    GameConfig config = plainConfig();
    config.color = true;
    config.bindings.push_back({'k', Command::MoveNorthWest});
    std::ostringstream output;
    auto console = std::make_unique<ScriptedConsole>(std::vector<int>{kEnter, 'k', 'Q'});
    ScriptedConsole* script = console.get();
    Game game(config, std::move(console), output);
    game.start();

    ASSERT_TRUE(game.getGauntlet().getPlayer().getPosition() == Position(5, 5));
    auto used = [&script](ConsoleColor color) {
        return std::find(script->colors.begin(), script->colors.end(), color) != script->colors.end();
    };
    ASSERT_TRUE(used(ConsoleColor::Red));     // The obstacle
    ASSERT_TRUE(used(ConsoleColor::Cyan));    // The EI tile
    ASSERT_TRUE(used(ConsoleColor::Magenta)); // Hazards
    ASSERT_TRUE(used(ConsoleColor::White));   // The player
    ASSERT_TRUE(used(ConsoleColor::Green));   // Full health
    ASSERT_TRUE(script->colors.back() == ConsoleColor::Default);

    // And none at all when colour is off
    std::ostringstream quiet_output;
    auto quiet = std::make_unique<ScriptedConsole>(std::vector<int>{kEnter, 's', 'Q'});
    ScriptedConsole* quiet_script = quiet.get();
    Game plain(plainConfig(), std::move(quiet), quiet_output);
    plain.start();
    ASSERT_TRUE(quiet_script->colors.empty());
    return true;
}

// Test case: the room's styled lines colour each kind of tile
bool testHud_StyledColors() {
    Gauntlet game(kSqlDir);
    std::vector<Hud::StyledLine> lines = Hud::renderStyledScreen(game);
    ASSERT_EQ(lines[1].text.substr(0, 15), "|......?......|");
    ASSERT_TRUE(lines[1].colors[7] == ConsoleColor::Red);      // ? Self-Doubt
    ASSERT_TRUE(lines[3].colors[3] == ConsoleColor::Cyan);     // E at (2,2)
    ASSERT_TRUE(lines[4].colors[4] == ConsoleColor::Magenta);  // : at (3,3)
    ASSERT_TRUE(lines[7].colors[7] == ConsoleColor::White);    // @ at (6,6)
    ASSERT_TRUE(lines[1].colors[1] == ConsoleColor::Default);  // Floor
    size_t health = lines[2].text.find("Health");
    ASSERT_TRUE(lines[2].colors[health] == ConsoleColor::Green);
    return true;
}

// Test case: a hit jolts the next frame one column to the right
bool testGame_ScreenShakesOnHits() {
    std::ostringstream output;
    // Enter, then north onto the hazard at (6,5), then wait
    auto console = std::make_unique<ScriptedConsole>(std::vector<int>{kEnter, 'w', 's'});
    Game game(plainConfig(), std::move(console), output);
    game.start();
    std::string text = output.str();
    size_t shaken = text.find(" +------^------+");
    ASSERT_TRUE(shaken != std::string::npos);
    ASSERT_TRUE(text.find("+------^------+", shaken + 2) != std::string::npos); // Then back in place
    return true;
}

// Test case: the event log and the profile report are written to the configured file
bool testGame_WritesEventLogAndProfile() {
    std::filesystem::path log = std::filesystem::temp_directory_path() / "quantapie_test.log";
    std::filesystem::remove(log);
    GameConfig config = plainConfig();
    config.log_file = log.string();
    config.profile = true;
    std::ostringstream output;
    Game game(config, std::make_unique<ScriptedConsole>(std::vector<int>{kEnter, 'w', 'Q'}), output);
    game.start();
    ASSERT_TRUE(game.getProfiler().getCalls("render") >= 2);
    ASSERT_TRUE(game.getProfiler().getCalls("input") >= 2);

    std::ifstream in(log);
    std::stringstream written;
    written << in.rdbuf();
    std::string text = written.str();
    ASSERT_TRUE(text.find("[turn 1] HazardHit: You step into a Rumination Loop: -3 health.") != std::string::npos);
    ASSERT_TRUE(text.find("Profile:") != std::string::npos);
    ASSERT_TRUE(text.find("render") != std::string::npos);
    in.close();
    std::filesystem::remove(log);
    return true;
}

// Test case: a Game with no world data fails clearly instead of starting
bool testGame_MissingDataDirectory() {
    bool threw = false;
    try {
        GameConfig config = plainConfig();
        config.data_directory = "/nonexistent/quantapie";
        std::ostringstream output;
        Game game(config, std::make_unique<ScriptedConsole>(std::vector<int>{}), output);
    } catch (const WorldDataError& error) {
        threw = std::string(error.what()).find("gauntlet_rooms.csv") != std::string::npos;
    }
    ASSERT_TRUE(threw);
    return true;
}

// Test case: config.txt settings, comments and errors
bool testGameConfig_ParsesSettings() {
    std::istringstream text(
        "# A comment\n"
        "data_directory = data\n"
        "save_directory = saves   # trailing comment\n"
        "background = steadfast\n"
        "mode = hardcore\n"
        "tutorial = off\n"
        "color = no\n"
        "log_file = run.log\n"
        "profile = on\n"
        "key.k = move_north\n"
        "key.# = wait\n"
        "key.= = dash\n"
        "\n");
    GameConfig config = GameConfig::parse(text);
    ASSERT_EQ(config.data_directory, "data");
    ASSERT_EQ(config.options.save_directory, "saves");
    ASSERT_TRUE(config.options.background == Background::Steadfast);
    ASSERT_TRUE(config.options.hardcore);
    ASSERT_TRUE(!config.options.tutorial);
    ASSERT_TRUE(!config.color);
    ASSERT_EQ(config.log_file, "run.log");
    ASSERT_TRUE(config.profile);
    ASSERT_EQ(config.bindings.size(), 3u);
    KeyBindings keys;
    config.applyBindings(keys);
    ASSERT_TRUE(keys.commandFor('k') == Command::MoveNorth);
    ASSERT_TRUE(keys.commandFor('#') == Command::Wait);
    ASSERT_TRUE(keys.commandFor('=') == Command::Dash);

    struct Bad {
        const char* text;
        const char* expected;
    };
    const Bad bad[] = {
        {"background = wizard\n", "config line 1: background must be balanced, steadfast, swift or grounded"},
        {"\nmode = easy\n", "config line 2: mode must be normal or hardcore"},
        {"color = maybe\n", "config line 1: unknown setting or bad value"},
        {"volume = 11\n", "config line 1: unknown setting or bad value: volume = 11"},
        {"key.k = fly\n", "config line 1: unknown command 'fly'"},
        {"just words\n", "config line 1: expected setting = value"},
    };
    for (const Bad& case_ : bad) {
        std::istringstream in(case_.text);
        std::string error;
        try {
            GameConfig::parse(in);
        } catch (const ConfigError& thrown) {
            error = thrown.what();
        }
        if (error.find(case_.expected) == std::string::npos) {
            std::cerr << "Expected \"" << case_.expected << "\", got \"" << error << "\"" << std::endl;
            return false;
        }
    }
    GameConfig defaults = GameConfig::loadFile("/nonexistent/config.txt");
    ASSERT_EQ(defaults.data_directory, "sql");
    ASSERT_TRUE(defaults.color);
    return true;
}

// Test case: the profiler counts and times named sections
bool testProfiler_RecordsSections() {
    Profiler profiler;
    for (int i = 0; i < 3; ++i) {
        Profiler::Scope timing(profiler, "work");
    }
    ASSERT_EQ(profiler.getCalls("work"), 3);
    ASSERT_EQ(profiler.getCalls("other"), 0);
    std::vector<std::string> report = profiler.report();
    ASSERT_EQ(report.size(), 1u);
    ASSERT_TRUE(report[0].find("work") == 0);
    ASSERT_TRUE(report[0].find("3 calls") != std::string::npos);
    return true;
}

// Function to register all console frontend tests with the runner
void registerGameTests(TestRunner& runner) {
    runner.addTest("testGame_ArrowKeysAndQuit", testGame_ArrowKeysAndQuit);
    runner.addTest("testGame_WelcomeMenuChoosesBackgroundAndMode", testGame_WelcomeMenuChoosesBackgroundAndMode);
    runner.addTest("testGame_WelcomeMenuLoadsASlot", testGame_WelcomeMenuLoadsASlot);
    runner.addTest("testGame_ClearsFirstRoomAndStopsAtEndOfInput", testGame_ClearsFirstRoomAndStopsAtEndOfInput);
    runner.addTest("testGame_RedrawsOnlyChangedLines", testGame_RedrawsOnlyChangedLines);
    runner.addTest("testGame_ColorsAndConfigBindings", testGame_ColorsAndConfigBindings);
    runner.addTest("testHud_StyledColors", testHud_StyledColors);
    runner.addTest("testGame_ScreenShakesOnHits", testGame_ScreenShakesOnHits);
    runner.addTest("testGame_WritesEventLogAndProfile", testGame_WritesEventLogAndProfile);
    runner.addTest("testGame_MissingDataDirectory", testGame_MissingDataDirectory);
    runner.addTest("testGameConfig_ParsesSettings", testGameConfig_ParsesSettings);
    runner.addTest("testProfiler_RecordsSections", testProfiler_RecordsSections);
}
