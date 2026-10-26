# Cross-platform build for Quanta_Pie (Windows and Linux).
#
#   cmake -S . -B build
#   cmake --build build
#   ctest --test-dir build          # run the tests
#   cmake --build build --target package   # a .zip (Windows) or .tar.gz (Linux) of the game and its data
#
# Run the game from the project root (or an unpacked package) so it finds sql/.
cmake_minimum_required(VERSION 3.16)
project(QuantaPie VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

# Everything but the programs' main() functions, shared by the game and the tests
file(GLOB QUANTAPIE_SOURCES CONFIGURE_DEPENDS
    src/objects/*.cpp
    src/players/*.cpp
    src/environments/*.cpp)
list(APPEND QUANTAPIE_SOURCES src/Game.cpp src/Room.cpp)
if(WIN32)
    list(APPEND QUANTAPIE_SOURCES src/platform/WindowsConsole.cpp)
else()
    list(APPEND QUANTAPIE_SOURCES src/platform/PosixConsole.cpp)
endif()

add_library(quantapie_core STATIC ${QUANTAPIE_SOURCES})
target_include_directories(quantapie_core PUBLIC src)

add_executable(quanta_pie src/main.cpp)
target_link_libraries(quanta_pie PRIVATE quantapie_core)

# Tests (Linux and Windows alike; nothing in them is platform-specific)
file(GLOB QUANTAPIE_TEST_SOURCES CONFIGURE_DEPENDS tests/*.cpp)
add_executable(quanta_pie_tests ${QUANTAPIE_TEST_SOURCES})
target_link_libraries(quanta_pie_tests PRIVATE quantapie_core)
target_compile_definitions(quanta_pie_tests PRIVATE QUANTAPIE_SQL_DIR="${CMAKE_CURRENT_SOURCE_DIR}/sql")

enable_testing()
add_test(NAME quanta_pie_tests COMMAND quanta_pie_tests)

# Packaging: the game, its world data and the README
install(TARGETS quanta_pie RUNTIME DESTINATION .)
install(DIRECTORY sql DESTINATION . FILES_MATCHING PATTERN "*.csv" PATTERN "*.sql")
install(FILES README.md LICENSE.md DESTINATION .)
if(WIN32)
    set(CPACK_GENERATOR ZIP)
else()
    set(CPACK_GENERATOR TGZ)
endif()
set(CPACK_PACKAGE_NAME "QuantaPie")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
include(CPack)
