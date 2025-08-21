#ifndef TOOL_H
#define TOOL_H

#include <string>

class Tool {
public:
    Tool(int id, const std::string& name, const std::string& description, int initial_room_id);

    int getId() const;
    std::string getName() const;
    std::string getDescription() const;
    int getInitialRoomId() const;

private:
    int id;
    std::string name;
    std::string description;
    int initial_room_id;
};

#endif // TOOL_H
