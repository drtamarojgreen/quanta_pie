#ifndef PROFILER_H
#define PROFILER_H

#include <chrono>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

/**
 * @class Profiler
 * @brief Measures how long each named part of the game takes, for a report at the end.
 *
 *   Profiler profiler;
 *   { Profiler::Scope timing(profiler, "render"); draw(); }
 *   for (const std::string& line : profiler.report()) { ... }
 */
class Profiler {
public:
    class Scope {
    public:
        Scope(Profiler& profiler, const std::string& section)
            : profiler(profiler), section(section), start(std::chrono::steady_clock::now()) {}
        ~Scope() { profiler.record(section, std::chrono::steady_clock::now() - start); }
        Scope(const Scope&) = delete;
        Scope& operator=(const Scope&) = delete;

    private:
        Profiler& profiler;
        std::string section;
        std::chrono::steady_clock::time_point start;
    };

    void record(const std::string& section, std::chrono::steady_clock::duration elapsed) {
        Totals& totals = sections[section];
        totals.calls++;
        totals.elapsed += elapsed;
    }

    long long getCalls(const std::string& section) const {
        auto found = sections.find(section);
        return found == sections.end() ? 0 : found->second.calls;
    }

    // One line per section: calls, total and average time in milliseconds
    std::vector<std::string> report() const {
        std::vector<std::string> lines;
        for (const auto& [section, totals] : sections) {
            double total_ms = std::chrono::duration<double, std::milli>(totals.elapsed).count();
            char line[160];
            std::snprintf(line, sizeof(line), "%-12s %8lld calls %10.3f ms total %8.4f ms each", section.c_str(),
                          totals.calls, total_ms, totals.calls ? total_ms / totals.calls : 0.0);
            lines.push_back(line);
        }
        return lines;
    }

private:
    struct Totals {
        long long calls = 0;
        std::chrono::steady_clock::duration elapsed{};
    };
    std::map<std::string, Totals> sections;
};

#endif // PROFILER_H
