# 🥧 Quanta_Pie

*A Text-Based C++ Adventure Game in the Quanta Series*

---

## 🌌 Overview

**Quanta_Pie** is the latest installment in the **Quanta Series**, a collection of experimental, lightweight projects that mix science, play, and creativity. This time, we’re diving into the deliciously mysterious world of text-based adventure.

In **Quanta_Pie**, you’ll navigate through a surreal landscape where logic, strategy, and curiosity matter more than graphics. Every choice alters the story, every action shifts the outcome, and the pie… well, the pie might just hold the key to everything.

Built entirely in **modern C++17**, Quanta_Pie is designed to be:

-   **🚀 Lightweight:** No unnecessary dependencies.
-   **⚡ Fast:** Text is instant, interaction is smooth.
-   **🧩 Expandable:** Modular code structure makes adding new story branches easy.

---

## ✨ Features

-   **Classic Text Adventure Gameplay:** Explore, choose, and survive in a branching narrative.
-   **Dynamic Story Paths:** Multiple endings based on your decisions.
-   **C++ Core:** Demonstrates clean, modern C++ techniques in a fun, narrative-driven project.
-   **Quanta Series Continuity:** Threads of scientific and philosophical themes tie back into the broader Quanta universe.

---

## 📈 Current Status

-   **In Development:** The game is currently in the active development phase.
-   **Tissdb Integration:** We are working on integrating Tissdb for persistent data storage and enhanced narrative branching.

---

## 🚀 Getting Started

### Build Instructions

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/your-username/Quanta_Pie.git
    cd Quanta_Pie
    ```

2.  **Compile the project:**
    To compile the game, you will need a C++ compiler that supports C++17. The following command uses `g++`:

    ```bash
    g++ -std=c++17 -o Quanta_Pie src/*.cpp
    ```
    *Note: In accordance with project directives, this command has not been tested but is expected to work based on the project structure.*

3.  **Run the game:**
    ```bash
    ./Quanta_Pie
    ```

---

## 🧪 Running Tests

A suite of unit and BDD-style tests is included in the `tests/` directory. To build and run the tests, you will need a C++ compiler that supports C++17.

1.  **Compile the test suite:**
    The following command uses `g++` to compile all test files and the necessary source files from the `src/` directory.

    ```bash
    g++ -std=c++17 -I./src -o test_runner tests/*.cpp src/*.cpp src/objects/*.cpp src/players/*.cpp
    ```
    *Note: In accordance with project directives, this command has not been tested but is expected to work based on the project structure.*

2.  **Run the tests:**
    ```bash
    ./test_runner
    ```

---

## 🤝 Contributing

Contributions are welcome! If you have ideas for new story branches, features, or improvements, feel free to open an issue or submit a pull request.

---

## 📜 License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.
