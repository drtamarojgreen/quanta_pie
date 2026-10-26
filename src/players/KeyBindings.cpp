#include "KeyBindings.h"
#include <utility>

namespace {

const std::pair<Command, const char*> kCommandNames[] = {
    {Command::MoveNorth, "move_north"},
    {Command::MoveNorthEast, "move_northeast"},
    {Command::MoveEast, "move_east"},
    {Command::MoveSouthEast, "move_southeast"},
    {Command::MoveSouth, "move_south"},
    {Command::MoveSouthWest, "move_southwest"},
    {Command::MoveWest, "move_west"},
    {Command::MoveNorthWest, "move_northwest"},
    {Command::Wait, "wait"},
    {Command::Look, "look"},
    {Command::Dash, "dash"},
    {Command::Clear, "clear"},
    {Command::Pause, "pause"},
    {Command::ShowInventory, "inventory"},
    {Command::ShowStats, "stats"},
    {Command::ShowBestiary, "bestiary"},
    {Command::ShowHelp, "help"},
    {Command::Save, "save"},
    {Command::Load, "load"},
    {Command::NewGame, "new_game"},
    {Command::Quit, "quit"},
};

bool isBindable(char key) {
    return key > ' ' && key < 127;
}

std::string trim(const std::string& text) {
    size_t begin = text.find_first_not_of(" \t\r");
    if (begin == std::string::npos) {
        return "";
    }
    size_t end = text.find_last_not_of(" \t\r");
    return text.substr(begin, end - begin + 1);
}

} // namespace

KeyBindings::KeyBindings() {
    resetToDefaults();
}

void KeyBindings::resetToDefaults() {
    bindings.fill(Command::None);
    bind('w', Command::MoveNorth);
    bind('e', Command::MoveNorthEast);
    bind('d', Command::MoveEast);
    bind('c', Command::MoveSouthEast);
    bind('x', Command::MoveSouth);
    bind('z', Command::MoveSouthWest);
    bind('a', Command::MoveWest);
    bind('q', Command::MoveNorthWest);
    bind('s', Command::Wait);
    bind('l', Command::Look);
    bind('r', Command::Dash);
    bind('f', Command::Clear);
    bind('p', Command::Pause);
    bind('i', Command::ShowInventory);
    bind('t', Command::ShowStats);
    bind('b', Command::ShowBestiary);
    bind('h', Command::ShowHelp);
    bind('S', Command::Save);
    bind('L', Command::Load);
    bind('N', Command::NewGame);
    bind('Q', Command::Quit);
}

void KeyBindings::bind(char key, Command command) {
    if (isBindable(key)) {
        bindings[static_cast<unsigned char>(key)] = command;
    }
}

void KeyBindings::unbind(char key) {
    bind(key, Command::None);
}

Command KeyBindings::commandFor(char key) const {
    return isBindable(key) ? bindings[static_cast<unsigned char>(key)] : Command::None;
}

std::vector<char> KeyBindings::keysFor(Command command) const {
    std::vector<char> keys;
    for (size_t key = 0; key < bindings.size(); ++key) {
        if (bindings[key] == command && command != Command::None) {
            keys.push_back(static_cast<char>(key));
        }
    }
    return keys;
}

bool KeyBindings::load(std::istream& config) {
    std::array<Command, 128> updated = bindings;
    std::string line;
    while (std::getline(config, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        size_t equals = line.find('=');
        if (equals == std::string::npos) {
            return false;
        }
        std::string key = trim(line.substr(0, equals));
        std::string name = trim(line.substr(equals + 1));
        Command command = commandFromName(name);
        if (key.size() != 1 || !isBindable(key[0]) || (command == Command::None && name != "none")) {
            return false;
        }
        updated[static_cast<unsigned char>(key[0])] = command;
    }
    bindings = updated;
    return true;
}

std::string KeyBindings::commandName(Command command) {
    for (const auto& [value, name] : kCommandNames) {
        if (value == command) {
            return name;
        }
    }
    return "none";
}

Command KeyBindings::commandFromName(const std::string& name) {
    for (const auto& [value, command_name] : kCommandNames) {
        if (name == command_name) {
            return value;
        }
    }
    return Command::None;
}

bool KeyBindings::directionFor(Command command, Direction& direction) {
    switch (command) {
        case Command::MoveNorth:     direction = Direction::North; return true;
        case Command::MoveNorthEast: direction = Direction::NorthEast; return true;
        case Command::MoveEast:      direction = Direction::East; return true;
        case Command::MoveSouthEast: direction = Direction::SouthEast; return true;
        case Command::MoveSouth:     direction = Direction::South; return true;
        case Command::MoveSouthWest: direction = Direction::SouthWest; return true;
        case Command::MoveWest:      direction = Direction::West; return true;
        case Command::MoveNorthWest: direction = Direction::NorthWest; return true;
        default: return false;
    }
}
