# QuantaVerse: Integration Specialist

QuantaVerse is a text-based adventure game that incorporates Cognitive Behavioral Therapy (CBT) principles into its gameplay. The player takes on the role of an "Integration Specialist" working through 100 rooms, each guarded by emotional obstacles that take Emotional Intelligence (EI) and Resilience (R) to overcome.

## Prerequisites

- A C++17 compiler: `g++` 9 or later (MinGW-w64 on Windows), `clang++`, or Visual Studio 2019 or later.
- Linux: `make`. Windows: nothing else for the one-line `g++` build.
- Optional, on either platform: CMake 3.16 or later.

The game reads its world from the CSV tables in `sql/` at startup (see [Game Data](#game-data)), so **always run it from the project root**.

## Building

### Linux (make)

```sh
make -C tests quanta_pie     # builds the game as tests/quanta_pie
```

### Windows (MinGW-w64 g++)

Open a terminal (PowerShell, Command Prompt or Git Bash) in the project root:

```sh
g++ src/main.cpp src/Game.cpp src/Room.cpp src/platform/WindowsConsole.cpp src/objects/*.cpp src/players/*.cpp src/environments/*.cpp -Isrc -std=c++17 -o quanta_pie.exe
```

### Either platform (CMake)

```sh
cmake -S . -B build
cmake --build build                      # builds quanta_pie (quanta_pie.exe on Windows) and quanta_pie_tests
cmake --build build --target package     # optional: a .zip (Windows) or .tar.gz (Linux) with the game, sql/ and this README
```

With Visual Studio, add `--config Release` to the build commands; the programs go in `build/Release/`.

Linux and Windows run the same `Game`; only the console differs (`PosixConsole` or `WindowsConsole`).

## Playing

From the project root:

```sh
make -C tests play           # Linux: build and run
./tests/quanta_pie           # Linux, once built (or ./build/quanta_pie with CMake)
quanta_pie.exe               # Windows (or build\quanta_pie.exe with CMake)
```

### Starting a run

The welcome screen lets you:

- Pick a background with `1`–`4`:
  - **Balanced:** 100 health, dash every 5 turns.
  - **Steadfast:** 130 health, dash every 8 turns.
  - **Swift:** 80 health, dash every 2 turns.
  - **Grounded:** 100 health, dash every 5 turns, never shaken.
- Toggle **hardcore** with `H`.
- Continue a saved game with `L` and a slot number.

Press Enter to begin.

### How it works

- **Every room is harder than the last.** Room *n* is guarded by ⌈√n⌉ obstacles that need *n* EI and R in total, from one Self-Doubt in room 1 to ten Burnouts in room 100.
- **Obstacles are the enemies.** Types 1–5 need 1–5 EI; types 6–10 need 5 EI plus 1–5 R. They stand along the north wall, the last one on the door, and are cleared one at a time, easiest first.
- **Collect, then clear.** The room's fixed spawn tiles hold exactly the EI and R the *next* obstacle needs. Walk onto the `E` and `R` tiles, stand next to the obstacle and press `f`. Clearing spends the EI and R, restores **10 + 3 × type** health, and brings out new tiles for the next obstacle. When the last one is cleared, the door opens.
- **Every move costs 1 health.** Hazards (`:`) cost 3 more. A dash moves two tiles for the cost of one move, then needs a few turns to recover.
- **Don't attack unprepared.** Pressing `f` without enough EI or R makes the obstacle strike back (2 × type health) and leaves you **shaken**: moves cost 2 until you wait (`s`) to breathe, or 5 turns pass.
- **Collapsing.** At 0 health you collapse. Normally you come to at the room's entry with full health and 25 less score. On **hardcore** the run ends and its save is deleted.
- **Endings.** *Unbroken* if you never collapsed, *Rising Again* if you got back up, *Rest* if a hardcore run ends.

A careful route never runs out of health; the test suite proves it by playing all 100 rooms. Wandering does run out.

### Map symbols

| Symbol | Meaning | | Symbol | Obstacle | Needs |
| --- | --- | --- | --- | --- | --- |
| `@` | You | | `?` | Self-Doubt | 1 EI |
| `E` | Emotional Intelligence | | `!` | Impulsive Anger | 2 EI |
| `R` | Resilience | | `~` | Misunderstanding | 3 EI |
| `:` | Hazard | | `z` | Apathy | 4 EI |
| `^` `v` | North door, south entry | | `&` | Interpersonal Conflict | 5 EI |
| `.` | Floor | | `<` | Setback | 5 EI + 1 R |
| `#` | Cleared room (mini-map) | | `%` | Sudden Change | 5 EI + 2 R |
| | | | `=` | Repeated Failure | 5 EI + 3 R |
| | | | `o` | Isolation | 5 EI + 4 R |
| | | | `*` | Burnout | 5 EI + 5 R |

The HUD shows:

- **Health:** with the change from your last action, coloured green, yellow or red as it falls.
- **EI and R:** what you carry against what the next obstacle needs.
- **The room's obstacles:** in clearing order, with the next one marked.
- **Progress:** score, turns, play time, dash readiness and status.
- **A 100-room mini-map:** each room ahead is shown by its hardest obstacle's symbol.
- **The current objective and a message log.**

### Controls

| Keys | Action |
| --- | --- |
| Arrow keys, Home/End/PgUp/PgDn, or `q w e` / `a d` / `z x c` | Move (including diagonally) |
| `s` | Wait a turn (and breathe off being shaken) |
| `l` then a direction | Look without moving |
| `r` then a direction | Dash two tiles |
| `f` | Clear the next obstacle (stand next to it) |
| `p` | Pause / resume (the timer stops too) |
| `i`, `t`, `b`, `h` | Inventory, statistics and achievements, bestiary, help |
| `S` then `1`–`3` | Save to a slot |
| `L` then `1`–`3` | Load a slot |
| `N` | New run, once this one is over |
| `Q` | Quit (`Ctrl+C` also works) |

Saves are written as `quantapie_slot1.sav` to `quantapie_slot3.sav` in the save directory (the project root unless `config.txt` says otherwise). A hardcore run keeps a single save and cannot be reloaded mid-run.

### Settings (`config.txt`)

Every setting is optional. Put a `config.txt` in the project root to change them:

```ini
# Lines starting with # are comments
data_directory = sql          # where the gauntlet_*.csv tables are
save_directory = .            # where save slots are written
background = balanced         # balanced, steadfast, swift or grounded
mode = normal                 # normal, or hardcore (permadeath)
tutorial = on                 # one-time tips as new mechanics come up
color = on                    # coloured text
log_file = quantapie.log      # append every game event to this file (no log when empty)
profile = off                 # time rendering and input, and add the report to the log
key.k = move_north            # rebind a key; command names are listed on the help screen
```

A bad line stops the game at startup with its line number and the problem.

## Testing

```sh
make -C tests                # Linux: build and run the tests (same as make -C tests test)
ctest --test-dir build       # CMake, on either platform, after building
```

The test program exits with status 1 if any test fails.

The tests cover:

- **A full playthrough:** all 100 rooms played through key presses on a careful route that never collapses, ending *Unbroken*.
- **Difficulty:** every room harder than the last.
- **Rules:** fixed spawn points, clearing in order, health, hazards, strikes, being shaken, the dash, collapse and hardcore permadeath.
- **Saving:** save slots and save/load (including corrupt files), and the timer.
- **Screens and frontend:** the bestiary, tips, the HUD and colours, and the console frontend (driven by a scripted console).
- **Settings and data:** `config.txt`, the profiler, and rejection of bad world data.

To also check for memory errors and undefined behaviour on Linux:

```sh
make -C tests clean
make -C tests CXXFLAGS="-std=c++17 -Wall -Wextra -fsanitize=address,undefined -g"
make -C tests clean
```

## Game Data

The world is defined by these tables in `sql/` (their schema is documented in `sql/game_data.sql`):

| File | Defines |
| --- | --- |
| `gauntlet_rooms.csv` | Each room's name, description and layout |
| `gauntlet_room_obstacles.csv` | Each room's obstacles, in the order they are cleared |
| `gauntlet_layouts.csv` | Room grids: size, north door tile and south entry tile |
| `gauntlet_spawn_points.csv` | The fixed EI and R spawn tiles of each layout, by slot |
| `gauntlet_obstacle_points.csv` | Where each layout places every obstacle but the last, which stands on the door |
| `gauntlet_hazards.csv` | Fixed hazard tiles in each layout and the health they cost |
| `gauntlet_exits.csv` | Connections between rooms (`north` through the door, `south` through the entry) |
| `gauntlet_obstacles.csv` | The map symbol, name and description of each obstacle type |
| `gauntlet_equipment.csv` | The EI or R equipment that spawns in each slot |

Some rules the loader enforces:

- An obstacle that needs N EI spawns EI slots 1 to N of its room's layout, and likewise for R.
- Each obstacle type needs its own symbol, and it can't be one the map already uses (`.@ER#+-|^v:`).
- Points and hazards can't share tiles with each other, the door or the entry.

If a table is missing or inconsistent, the game stops at startup and names the file, line and problem.

## Roadmap status

**From `docs/enhancements.md`, implemented:**

| Section | Items |
| --- | --- |
| Engine | 1 incremental redraw, 3 state machine, 5 everything allocated up front, 6 event queue, 8 `config.txt`, 9 modular loop, 10 profiler |
| Controls | 11 key bindings, 12 dash with cooldown, 14 look, 18 wait, 19 diagonal moves |
| Mechanics | 21 collisions, 26 shaken status, 28 and 40 hazards |
| World | 32, 35 (named zones), 36 |
| Items | 41, 42, 43 |
| UI | 61 HUD with health, 62 damage and healing numbers, 63, 64, 65, 66 colour, 67, 68 screen shake on hits, 69 status display, 70 look shows each obstacle's needs |
| Game structure | 91, 92 three slots, 94, 95, 96 tips, 97 hardcore, 98 timer, 99 bestiary, 100 backgrounds |

**From `docs/plan.md`, implemented:**

- The game loop, state and modular structure.
- Data-driven descriptions and challenges.
- Three endings, chosen by how you played.
- Inventory and stats.
- Automated playthroughs and balance checked by simulation.
- The CMake build with packaging, and these instructions.

**Not integrated, and why:**

- **2** (fixed time-step) and **20** (charge): play is turn-based, one key at a time.
- **7** (seeded randomness), **33** (moving terrain) and **37** (procedural layouts): spawn points and layouts are fixed in the data.
- **13** (stamina) and **23** (hunger): health is already the drain on every move.
- **15**, **21 gravity** and **39**: the rooms are a flat grid.
- **16** (stealth): obstacles don't move or detect.
- **17** (combos): each clear needs fresh equipment.
- **22**, **24**, **25**, **29**, **31**, **48**, **49**, **50** and **93**: each would add, save or discount EI/R, or let you skip an obstacle's fixed cost.
- **27**: no NPCs.
- **30**: EI and R are already the ability resource.
- **34**: nothing for weather to act on.
- **38**: the single path from room 1 to 100 keeps each room harder than the last.
- **44**–**47**: no weapons, ammo or shop.

From the plan:

- **Typed text commands:** single-key input.
- **Branching story:** one path.
- **Playtesting with people:** needs players.
- **A v1.0 tag:** left to the maintainers.
