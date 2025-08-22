# QuantaVerse: Integration Specialist

QuantaVerse is a text-based adventure game that incorporates Cognitive Behavioral Therapy (CBT) principles into its gameplay. The player takes on the role of an "Integration Specialist" navigating challenges and interactions within a futuristic world.

## Prerequisites

To build and run this project, you will need a C++ compiler that supports C++11 or later (for features like `std::function` and lambdas). The `g++` compiler, part of the MinGW-w64 toolchain on Windows or available on Linux/macOS, is a standard choice.

## Building the Application

1.  **Open a terminal** or command prompt (like PowerShell, Command Prompt, or Git Bash) and navigate to the root directory of this project (`quanta_pie`).

2.  **Compile the source files** by running the following command. This command gathers all the necessary `.cpp` files, tells the compiler where to find the header files (with `-Isrc`), and links them into a single executable.

    ```sh
    g++ src/main.cpp src/Game.cpp src/objects/*.cpp -o quanta_pie.exe -Isrc -std=c++17
    ```
    *Note 1: We use `src/objects/*.cpp` to automatically include all game object source files (like `Challenge.cpp`, `Player.cpp`, etc.) from the `src/objects` directory.*
    *Note 2: We use `-std=c++17` to enable modern C++ features like `std::make_unique` and structured bindings.*

3.  After the command completes successfully, you should see a new file named `quanta_pie.exe` in the project's root directory.

## Running the Game

Once the application is built, you can run it from the same terminal:

```sh
./quanta_pie.exe
```

The game will start, and you can interact with it through the console. To exit the game at any time, press `Ctrl+C`.