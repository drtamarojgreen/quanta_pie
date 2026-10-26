# Builds and runs the test suite on Linux: make -C tests
# Builds and runs the game on Linux:       make -C tests play
CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra
CPPFLAGS += -I../src -DQUANTAPIE_SQL_DIR='"$(abspath ../sql)"'

# Everything except the Windows console and the programs' main() functions
GAME_SOURCES = ../src/Game.cpp \
               ../src/Room.cpp \
               ../src/platform/PosixConsole.cpp \
               $(wildcard ../src/objects/*.cpp) \
               $(wildcard ../src/players/*.cpp) \
               $(wildcard ../src/environments/*.cpp)
SOURCES = $(wildcard *.cpp) $(GAME_SOURCES)

TARGET = run_tests
GAME = quanta_pie

.PHONY: all test play clean

all: test

$(TARGET): $(SOURCES) $(wildcard *.h ../src/*.h ../src/*/*.h)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(SOURCES) -o $@

test: $(TARGET)
	./$(TARGET)

$(GAME): ../src/main.cpp $(GAME_SOURCES) $(wildcard ../src/*.h ../src/*/*.h)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) ../src/main.cpp $(GAME_SOURCES) -o $@

# The game reads sql/ relative to where it runs, like the Windows build
play: $(GAME)
	cd .. && ./tests/$(GAME)

clean:
	rm -f $(TARGET) $(GAME)
