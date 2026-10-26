#include "GameConfig.h"
#include <fstream>

namespace {

std::string trim(const std::string& text) {
    size_t begin = text.find_first_not_of(" \t\r");
    if (begin == std::string::npos) {
        return "";
    }
    size_t end = text.find_last_not_of(" \t\r");
    return text.substr(begin, end - begin + 1);
}

bool onOff(const std::string& value, bool& result) {
    if (value == "on" || value == "true" || value == "yes") {
        result = true;
        return true;
    }
    if (value == "off" || value == "false" || value == "no") {
        result = false;
        return true;
    }
    return false;
}

} // namespace

const char* const GameConfig::kDefaultFile = "config.txt";

GameConfig GameConfig::parse(std::istream& in) {
    GameConfig config;
    std::string line;
    int number = 0;
    while (std::getline(in, line)) {
        number++;
        line = trim(line);
        // "key.# = ..." rebinds the # key, so a comment can only start after the key name.
        bool binding = line.compare(0, 4, "key.") == 0;
        size_t comment = line.find('#', binding ? 5 : 0);
        if (comment != std::string::npos) {
            line = trim(line.substr(0, comment));
        }
        if (line.empty()) {
            continue;
        }
        std::string where = "config line " + std::to_string(number) + ": ";
        size_t equals = line.find('=', binding ? 5 : 0); // "key.= = ..." rebinds the = key
        if (equals == std::string::npos) {
            throw ConfigError(where + "expected setting = value");
        }
        std::string key = trim(line.substr(0, equals));
        std::string value = trim(line.substr(equals + 1));
        bool flag = false;
        if (key == "data_directory" && !value.empty()) {
            config.data_directory = value;
        } else if (key == "save_directory" && !value.empty()) {
            config.options.save_directory = value;
        } else if (key == "background") {
            if (!backgroundFromName(value, config.options.background)) {
                throw ConfigError(where + "background must be balanced, steadfast, swift or grounded");
            }
        } else if (key == "mode") {
            if (value != "normal" && value != "hardcore") {
                throw ConfigError(where + "mode must be normal or hardcore");
            }
            config.options.hardcore = value == "hardcore";
        } else if (key == "tutorial" && onOff(value, flag)) {
            config.options.tutorial = flag;
        } else if (key == "color" && onOff(value, flag)) {
            config.color = flag;
        } else if (key == "profile" && onOff(value, flag)) {
            config.profile = flag;
        } else if (key == "log_file") {
            config.log_file = value;
        } else if (key.size() == 5 && key.compare(0, 4, "key.") == 0) {
            Command command = KeyBindings::commandFromName(value);
            if (command == Command::None && value != "none") {
                throw ConfigError(where + "unknown command '" + value + "'");
            }
            config.bindings.push_back({key[4], command});
        } else {
            throw ConfigError(where + "unknown setting or bad value: " + line);
        }
    }
    return config;
}

GameConfig GameConfig::loadFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        return GameConfig();
    }
    return parse(in);
}

void GameConfig::applyBindings(KeyBindings& keys) const {
    for (const auto& [key, command] : bindings) {
        keys.bind(key, command);
    }
}
