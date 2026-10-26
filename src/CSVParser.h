#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

namespace CSVParser {

    // Function to split a string by a delimiter, handling quoted fields.
    // Inside quotes, a doubled quote ("") stands for one literal quote.
    inline std::vector<std::string> split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string currentToken;
        bool inQuote = false;

        for (size_t i = 0; i < s.length(); ++i) {
            char c = s[i];

            if (c == '"') {
                if (inQuote && i + 1 < s.length() && s[i+1] == '"') {
                    currentToken += '"';
                    i++; // Skip the second quote of the escaped pair
                } else {
                    inQuote = !inQuote;
                }
            } else if (c == delimiter && !inQuote) {
                tokens.push_back(currentToken);
                currentToken = "";
            } else {
                currentToken += c;
            }
        }
        tokens.push_back(currentToken); // Add the last token
        return tokens;
    }

    // Function to read a CSV file and return its content as a vector of string vectors
    inline std::vector<std::vector<std::string>> readCSV(const std::string& filename) {
        std::vector<std::vector<std::string>> data;
        std::ifstream file(filename);

        if (!file.is_open()) {
            return data; // Return empty data if file cannot be opened
        }

        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back(); // Files saved on Windows end lines with \r\n
            }
            data.push_back(split(line, ','));
        }

        file.close();
        return data;
    }

} // namespace CSVParser

#endif // CSV_PARSER_H
