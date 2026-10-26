#ifndef MESSAGE_LOG_H
#define MESSAGE_LOG_H

#include <deque>
#include <string>

/**
 * @enum MessageTone
 * @brief How a message reads, so a HUD can colour it.
 */
enum class MessageTone {
    Neutral,
    Good, // Progress: pickups, clears, achievements
    Bad,  // Harm or failure: strikes, hazards, collapses
    Tip   // Tutorial tips
};

/**
 * @class MessageLog
 * @brief Keeps the most recent game messages for the HUD, discarding the oldest.
 */
class MessageLog {
public:
    explicit MessageLog(size_t capacity = 6) : capacity(capacity) {}

    void add(const std::string& message, MessageTone tone = MessageTone::Neutral) {
        messages.push_back(message);
        tones.push_back(tone);
        while (messages.size() > capacity) {
            messages.pop_front();
            tones.pop_front();
        }
    }

    void clear() {
        messages.clear();
        tones.clear();
    }
    const std::deque<std::string>& getMessages() const { return messages; }
    const std::deque<MessageTone>& getTones() const { return tones; } // Parallel to getMessages()
    size_t getCapacity() const { return capacity; }

private:
    size_t capacity;
    std::deque<std::string> messages;
    std::deque<MessageTone> tones;
};

#endif // MESSAGE_LOG_H
