#ifndef GAMESESSION_H
#define GAMESESSION_H

#include <string>

class GameSession {
public:
    GameSession(int id, const std::string& type, const std::string& start, const std::string& end)
        : session_id(id), game_type(type), start_time(start), end_time(end) {}

    int getSessionID() const { return session_id; }
    std::string getGameType() const { return game_type; }
    std::string getStartTime() const { return start_time; }
    std::string getEndTime() const { return end_time; }

private:
    int session_id;
    std::string game_type;
    std::string start_time;
    std::string end_time;
};

#endif // GAMESESSION_H
