# Plan: 3D Text-Based Rendering Engine

This document outlines the plan to transform the Quanta_Pie game from a classic room-based text adventure into a first-person perspective game with a 3D-like view rendered using ASCII characters, similar to early 3D games like Doom.

## 1. Core Architectural Changes

The existing architecture, based on discrete `Room` objects with string-based exits, is not suitable for a 3D world. It will be replaced with a 2D map-based system.

- **Action:** Deprecate or remove the current `Room` and exit-based navigation system.
- **Action:** Introduce a 2D map representation for the game world.

## 2. Phase 1: The 2D Map System

The foundation of the 3D view is a 2D map.

- **Component:** `Map` class (`Map.h`, `Map.cpp`).
- **Details:**
    - The `Map` will be represented by a `std::vector<std::string>` or a 2D array of characters/integers.
    - `'#'` will represent a wall.
    - `'.'` will represent an empty space.
    - The class will provide methods to get the character at a specific (x, y) coordinate and to check for walls.

## 3. Phase 2: Player Representation and Controls

The player's interaction with the world will change from text commands to movement-based controls.

- **Component:** `GamePlayer` class modifications.
- **Details:**
    - The player will have an `(x, y)` position as floating-point coordinates on the 2D map.
    - The player will have a `direction` vector and a `cameraPlane` vector to define their viewing angle and field of view.
    - The game loop will be updated to handle new controls:
        - **Move Forward/Backward:** Change player's (x, y) position along the direction vector.
        - **Turn Left/Right:** Rotate the player's direction vector and camera plane using rotation matrices.

## 4. Phase 3: The Raycasting Engine

This is the core of the 3D simulation. For each vertical stripe of the screen, we cast a ray to calculate what is seen.

- **Component:** `Raycaster` class or a set of functions within the `Game` class.
- **Algorithm:**
    1. For each `x` from 0 to `screenWidth`:
    2. Calculate the ray's position and direction based on the player's direction and the current screen column.
    3. Implement a Digital Differential Analysis (DDA) algorithm to step along the ray, checking for wall hits on the 2D map grid.
    4. Once a wall is hit, calculate the perpendicular distance from the player to the wall to avoid a "fisheye" effect.
    5. Based on the distance, calculate the height of the wall slice to be drawn in that screen column. A shorter distance means a taller wall.

## 5. Phase 4: The Text-Based Renderer

This component will translate the raycasting results into an ASCII art representation on the console.

- **Component:** `Renderer` class or functions.
- **Details:**
    - The renderer will manage a character buffer representing the screen.
    - For each screen column, it will draw the wall slice calculated by the raycaster.
    - The character used for the wall can be shaded based on distance (e.g., `#` for close, `=` for medium, `-` for far) or based on whether it's a north-south or east-west facing wall.
    - The floor and ceiling will be drawn with different characters (e.g., `.` for floor, ' ' for ceiling).
    - The renderer will need a function to clear the console and print the entire character buffer for each frame.

## 6. Integration and Game Loop

All components will be brought together in the main game loop.

- **Action:** Refactor `Game::gameLoop()`.
- **Details:**
    - The loop will continuously:
        1. Handle user input for movement and turning.
        2. Update the player's position and direction.
        3. For each frame, run the raycasting algorithm and render the scene to the console.

## 7. Future Enhancements (Post-MVP)

- **Textured Walls:** Use different characters for different wall types.
- **Sprites:** Add items and enemies as sprites within the 3D view.
- **Collision Detection:** Improve movement to prevent walking through walls.
