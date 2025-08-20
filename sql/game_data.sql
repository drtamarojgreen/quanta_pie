-- SQL script for TissDB game data.
--
-- This script assumes a schema for a simple game with players, sessions, and scores.
-- This script defines the schema and initial data for a simple game with
-- players, sessions, and scores.

-- =============================================================================
-- Schema Definitions
-- =============================================================================

CREATE TABLE players (
    player_id INT PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    join_date DATETIME
);

CREATE TABLE game_sessions (
    session_id INT PRIMARY KEY,
    game_type VARCHAR(255),
    start_time DATETIME,
    end_time DATETIME
);

CREATE TABLE scores (
    score_id INT PRIMARY KEY,
    player_id INT,
    session_id INT,
    score INT,
    FOREIGN KEY (player_id) REFERENCES players(player_id),
    FOREIGN KEY (session_id) REFERENCES game_sessions(session_id)
);


-- =============================================================================
-- Sample Data Insertion
-- =============================================================================

--
-- Insert data into the 'players' collection
--
INSERT INTO players (player_id, name, join_date) VALUES (1, 'Alice', '2023-01-15 10:30:00');
INSERT INTO players (player_id, name, join_date) VALUES (2, 'Bob', '2023-01-16 11:00:00');
INSERT INTO players (player_id, name, join_date) VALUES (3, 'Charlie', '2023-01-17 12:15:00');

--
-- Insert data into the 'game_sessions' collection
--
INSERT INTO game_sessions (session_id, game_type, start_time, end_time) VALUES (101, 'Chess', '2023-02-01 14:00:00', '2023-02-01 15:00:00');
INSERT INTO game_sessions (session_id, game_type, start_time, end_time) VALUES (102, 'Checkers', '2023-02-02 16:00:00', '2023-02-02 16:30:00');
INSERT INTO game_sessions (session_id, game_type, start_time, end_time) VALUES (103, 'Chess', '2023-02-03 18:00:00', '2023-02-03 19:30:00');

--
-- Insert data into the 'scores' collection
--
-- Scores for session 101 (Chess)
INSERT INTO scores (score_id, player_id, session_id, score) VALUES (1001, 1, 101, 10); -- Alice wins
INSERT INTO scores (score_id, player_id, session_id, score) VALUES (1002, 2, 101, 5);  -- Bob loses
-- Scores for session 102 (Checkers)
INSERT INTO scores (score_id, player_id, session_id, score) VALUES (1003, 2, 102, 20); -- Bob wins
INSERT INTO scores (score_id, player_id, session_id, score) VALUES (1004, 3, 102, 15); -- Charlie loses
-- Scores for session 103 (Chess)
INSERT INTO scores (score_id, player_id, session_id, score) VALUES (1005, 1, 103, 12); -- Alice wins
INSERT INTO scores (score_id, player_id, session_id, score) VALUES (1006, 3, 103, 8);  -- Charlie loses

--
-- Schema and data for Rooms
--
CREATE TABLE rooms (
    room_id INT PRIMARY KEY,
    description TEXT,
    ascii_art TEXT
);

INSERT INTO rooms (room_id, description, ascii_art) VALUES (1, 'You find yourself in a vast, circular library, its towering shelves carved from a single, colossal petrified tree. The air hums with a faint, magical resonance, and the only light filters down from glowing crystals embedded in the ceiling, illuminating a massive, rune-etched obsidian desk at the room''s heart.', '   _______\n  /      /,\n /      //\n/______//\n(______(/');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (2, 'The air is thick with the scent of ancient paper and preservation wards. You are in the archives, a labyrinth of impossibly tall shelves that disappear into the gloom above. Each shelf is crammed with scrolls, codices, and leather-bound tomes, their spines either blank or marked with cryptic symbols that seem to shift when you''re not looking directly at them.', '||||||||\n|      |\n|      |\n||||||||');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (3, 'Tucked away behind a tapestry depicting a forgotten battle, you discover a hidden nook. A plush, high-backed armchair sits before a fireplace where the flames burn a soothing, ethereal blue. A small, floating orb of light provides perfect illumination for reading, and the gentle crackling of the fire is the only sound.', '  )  (\n /`--`\\\n|      |\n|______|');


--
-- Schema and data for Items
--
CREATE TABLE items (
    item_id INT PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    description TEXT
);

INSERT INTO items (item_id, name, description) VALUES (1, 'Obsidian Key', 'A key carved from a single piece of volcanic glass. It feels cold to the touch.');
INSERT INTO items (item_id, name, description) VALUES (2, 'Tattered Scroll', 'A fragile scroll covered in faded, unreadable glyphs.');


--
-- Schema and data for Player Inventory
--
CREATE TABLE player_inventory (
    inventory_id INT PRIMARY KEY,
    player_id INT,
    item_id INT,
    quantity INT,
    FOREIGN KEY (player_id) REFERENCES players(player_id),
    FOREIGN KEY (item_id) REFERENCES items(item_id)
);

-- Give Alice the Tattered Scroll
INSERT INTO player_inventory (inventory_id, player_id, item_id, quantity) VALUES (1, 1, 2, 1);


--
-- Schema and data for Nexus Flow (Room Connectivity)
--
CREATE TABLE room_exits (
    exit_id INT PRIMARY KEY,
    source_room_id INT,
    destination_room_id INT,
    direction VARCHAR(255),
    description TEXT,
    required_item_id INT,
    FOREIGN KEY (source_room_id) REFERENCES rooms(room_id),
    FOREIGN KEY (destination_room_id) REFERENCES rooms(room_id),
    FOREIGN KEY (required_item_id) REFERENCES items(item_id)
);

-- An open exit from the Library (1) to the Archives (2)
INSERT INTO room_exits (exit_id, source_room_id, destination_room_id, direction, description, required_item_id) VALUES (1, 1, 2, 'EAST', 'A grand, marble archway leads to the east.', NULL);
-- A locked exit from the Library (1) to the Hidden Nook (3)
INSERT INTO room_exits (exit_id, source_room_id, destination_room_id, direction, description, required_item_id) VALUES (2, 1, 3, 'WEST', 'A heavy, oak door is set into the western wall. It has a large, obsidian lock.', 1);
-- The return exit from the Archives (2) to the Library (1)
INSERT INTO room_exits (exit_id, source_room_id, destination_room_id, direction, description, required_item_id) VALUES (3, 2, 1, 'WEST', 'The marble archway leads back to the main library.', NULL);
