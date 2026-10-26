#include "PosixConsole.h"
#include <cstdio>
#include <iostream>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

namespace {

const int kExtendedKeyPrefix = 224;

// Puts the terminal in single-key, no-echo mode for its lifetime, if stdin is a terminal.
class RawMode {
public:
    RawMode() : active(tcgetattr(STDIN_FILENO, &saved) == 0) {
        if (active) {
            termios raw = saved;
            raw.c_lflag &= ~(ICANON | ECHO);
            raw.c_cc[VMIN] = 1;
            raw.c_cc[VTIME] = 0;
            tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        }
    }
    ~RawMode() {
        if (active) {
            tcsetattr(STDIN_FILENO, TCSANOW, &saved);
        }
    }

    // Reads one byte, giving up after timeout_tenths of a second when in raw mode.
    int readByte(int timeout_tenths = 0) {
        if (active && timeout_tenths > 0) {
            termios timed;
            tcgetattr(STDIN_FILENO, &timed);
            timed.c_cc[VMIN] = 0;
            timed.c_cc[VTIME] = static_cast<cc_t>(timeout_tenths);
            tcsetattr(STDIN_FILENO, TCSANOW, &timed);
        }
        unsigned char byte = 0;
        ssize_t count = read(STDIN_FILENO, &byte, 1);
        return count == 1 ? byte : -1;
    }

private:
    termios saved;
    bool active;
};

// Maps the final byte of an ESC [ x sequence to the matching Windows scan code.
int scanCodeFor(int final_byte) {
    switch (final_byte) {
        case 'A': return 72; // Up
        case 'B': return 80; // Down
        case 'D': return 75; // Left
        case 'C': return 77; // Right
        case 'H': return 71; // Home
        case 'F': return 79; // End
        default:  return -1;
    }
}

} // namespace

PosixConsole::PosixConsole() : pending_scan_code(-1) {}

void PosixConsole::clear() {
    std::cout << "\033[2J\033[H" << std::flush;
}

void PosixConsole::setCursorPosition(int x, int y) {
    std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H";
}

int PosixConsole::getChar() {
    if (pending_scan_code >= 0) {
        int scan_code = pending_scan_code;
        pending_scan_code = -1;
        return scan_code;
    }
    std::cout << std::flush;
    RawMode raw;
    int ch = raw.readByte();
    if (ch == '\033') {
        // An arrow key arrives as ESC [ A; a lone ESC times out after 0.1s.
        int bracket = raw.readByte(1);
        if (bracket != '[' && bracket != 'O') {
            return ch;
        }
        int scan_code = scanCodeFor(raw.readByte(1));
        if (scan_code < 0) {
            return ch;
        }
        pending_scan_code = scan_code;
        return kExtendedKeyPrefix;
    }
    return ch == '\n' ? '\r' : ch; // Windows' _getch() reports Enter as '\r'
}

int PosixConsole::getConsoleWidth() const {
    winsize size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0 && size.ws_col > 0) {
        return size.ws_col;
    }
    return 80;
}

void PosixConsole::setColor(ConsoleColor color) {
    switch (color) {
        case ConsoleColor::Default: std::cout << "\033[0m"; break;
        case ConsoleColor::Red:     std::cout << "\033[31m"; break;
        case ConsoleColor::Green:   std::cout << "\033[32m"; break;
        case ConsoleColor::Yellow:  std::cout << "\033[33m"; break;
        case ConsoleColor::Blue:    std::cout << "\033[34m"; break;
        case ConsoleColor::Magenta: std::cout << "\033[35m"; break;
        case ConsoleColor::Cyan:    std::cout << "\033[36m"; break;
        case ConsoleColor::White:   std::cout << "\033[1;37m"; break;
    }
}
