-- SQL script for TissDB game data.
--
-- This script assumes a schema for a simple game with players, sessions, and scores.
-- Since TissDB does not support DDL statements like CREATE TABLE, this script
-- only contains INSERT statements to populate the collections. The collections
-- are assumed to be created beforehand via the TissDB API.
--
-- Assumed Schema:
--
-- Collection: players
-- Description: Stores information about each player.
-- Fields:
--   - player_id (NUMBER, PRIMARY KEY)
--   - name (STRING)
--   - join_date (DATETIME)
--
-- Collection: game_sessions
-- Description: Stores information about each game session.
-- Fields:
--   - session_id (NUMBER, PRIMARY KEY)
--   - game_type (STRING) - e.g., 'Chess', 'Checkers'
--   - start_time (DATETIME)
--   - end_time (DATETIME)
--
-- Collection: scores
-- Description: Stores the scores for each player in each game session.
-- Fields:
--   - score_id (NUMBER, PRIMARY KEY)
--   - player_id (NUMBER, FOREIGN KEY to players.player_id)
--   - session_id (NUMBER, FOREIGN KEY to game_sessions.session_id)
--   - score (NUMBER)
--

-- =============================================================================
-- Insert Sample Data
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
-- Assumed Schema for Rooms
--
-- Collection: rooms
-- Description: Stores information about each room in the game world.
-- Fields:
--   - room_id (NUMBER, PRIMARY KEY)
--   - description (STRING)
--   - ascii_art (STRING)
--

--
-- Insert data into the 'rooms' collection
--
INSERT INTO rooms (room_id, description, ascii_art) VALUES (1, 'You find yourself in a vast, circular library, its towering shelves carved from a single, colossal petrified tree. The air hums with a faint, magical resonance, and the only light filters down from glowing crystals embedded in the ceiling, illuminating a massive, rune-etched obsidian desk at the room''s heart.', '   _______\n  /      /,\n /      //\n/______//\n(______(/');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (2, 'The air is thick with the scent of ancient paper and preservation wards. You are in the archives, a labyrinth of impossibly tall shelves that disappear into the gloom above. Each shelf is crammed with scrolls, codices, and leather-bound tomes, their spines either blank or marked with cryptic symbols that seem to shift when you''re not looking directly at them.', '||||||||\n|      |\n|      |\n||||||||');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (3, 'Tucked away behind a tapestry depicting a forgotten battle, you discover a hidden nook. A plush, high-backed armchair sits before a fireplace where the flames burn a soothing, ethereal blue. A small, floating orb of light provides perfect illumination for reading, and the gentle crackling of the fire is the only sound.', '  )  (\n /`--`\\\n|      |\n|______|');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (4, 'You stand in a cavernous grand hall, its vaulted ceiling lost in shadow. Massive stone pillars, carved with the likenesses of ancient kings, support the immense weight above. A long, dust-covered banquet table, capable of seating a hundred, stretches down the center of the hall. The air is still and heavy with the weight of forgotten history.', '   | |         | |\n|  | | |     | | |  |\n|  | | |     | | |  |\n|__| |_|_____|_| |__|\n(____________________)');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (5, 'A surprisingly well-preserved kitchen adjoins the grand hall. A huge, soot-blackened fireplace dominates one wall, with a massive iron cauldron hanging in its hearth. Copper pots and pans, tarnished with age, hang from hooks on the walls. The faint, ghostly smell of a long-ago feast lingers in the air.', ',-----------.\n|]         [|\n|           |\n|___________|\n|o|       |o|\n|=|       |=|');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (6, 'You''ve stumbled into a dark, unsettling chamber. The air is cold and carries a metallic tang. Rusted chains hang from the ceiling, and various grim-looking implements of pain are mounted on the walls. In the center of the room is a wooden rack, its purpose all too clear. A single drain in the floor hints at the room''s gruesome past.', '/`-.--`-.\n| o| o |\n`-/`--`\\\n  |     |\n /|     |\\\n/ |     | \\\n`=` `---` `=`');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (7, 'A spiral staircase leads you up to a circular room at the top of a tower. The ceiling is a magnificent dome of crystal, offering a breathtaking, if magically distorted, view of a swirling cosmos. A massive, brass-and-iron telescope, aimed at a dead star, dominates the room''s center.', '   .---.\n  /     \\\n |       |\n |   o   |\n/|/\\   /|\\\n( |  `=`  | )\n`|_______|`');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (8, 'This room is clearly an armory. Racks of spears, swords, and axes line the walls, though most are pitted with rust. A few suits of plate armor stand silent sentinel in the corners, their visors like empty eyes staring into nothingness. The smell of oil and old metal is strong here.', '|` |` |` |\n|-`|-`|-`|\n|  |  |  |\n(-----)  |\n| | | |  |\n|_|_|_|__|');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (9, 'A chaotic laboratory greets you, filled with the pungent smell of strange chemicals. Benches are cluttered with alembics, retorts, and beakers containing colorful, bubbling liquids. Complicated charts of celestial bodies and anatomical diagrams adorn the walls. A thick layer of dust covers everything, suggesting the lab was abandoned in a hurry.', '  .---.\n (     )\n  `---`\n /  |  \\\n(   |   )\n `--`--`');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (10, 'You enter a natural cavern, its floor submerged in a pool of crystal-clear, unnaturally still water. Glowing fungi cast a soft, blue-green light across the chamber, revealing intricate stalactites and stalagmites. The sound of a slow, steady drip echoes in the silence.', ',^,^,^,^,\n/         \\\n(           )\n~~~~~~~~~~~~~~~~~\n~~~~~~~~~~~~~~~~~\n(___________)');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (11, 'This is undoubtedly the throne room. A raised dais supports a massive, imposing throne carved from a single piece of obsidian, inlaid with veins of gold. Tattered banners, their sigils long faded, hang from the walls. A sense of immense, forgotten power permeates the room.', '  |---|\n /     \\\n|  _|_  |\n| (___) |\n|_______|');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (12, 'You are in a long, curved hallway with a domed ceiling. The acoustics are strange; the slightest sound seems to travel along the walls, distorted into a faint whisper. The walls themselves are smooth, featureless stone, offering no clue as to the gallery''s purpose.', '/`--------`\\\n/           \\\n/             \\\n(               )\n(               )\n\\             /\n \\           /\n  `---------`');

--
-- Assumed Schema for Maps
--
-- Collection: maps
-- Description: Stores connections (exits) between rooms.
-- Fields:
--   - map_id (NUMBER, PRIMARY KEY)
--   - from_room_id (NUMBER, FOREIGN KEY to rooms.room_id)
--   - to_room_id (NUMBER, FOREIGN KEY to rooms.room_id)
--   - direction (STRING)
--   - description (STRING)
--

--
-- Insert data into the 'maps' collection
--
-- Library <-> Archives
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (1, 1, 2, 'north', 'An ornate archway leads to the archives.');
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (2, 2, 1, 'south', 'An ornate archway leads back to the library.');
-- Library <-> Reading Nook
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (3, 1, 3, 'east', 'A heavy tapestry conceals a passage to a quiet nook.');
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (4, 3, 1, 'west', 'A heavy tapestry conceals a passage back to the library.');
-- Library <-> Grand Hall
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (5, 1, 4, 'down', 'A grand staircase descends into the hall below.');
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (6, 4, 1, 'up', 'A grand staircase ascends to the library.');
-- Grand Hall <-> Kitchen
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (7, 4, 5, 'east', 'A swinging door leads into the kitchen.');
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (8, 5, 4, 'west', 'A swinging door leads back to the grand hall.');
-- Grand Hall <-> Armory
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (9, 4, 8, 'north', 'A reinforced door opens into the armory.');
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (10, 8, 4, 'south', 'A reinforced door opens back into the grand hall.');
-- Grand Hall <-> Throne Room
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (11, 4, 11, 'south', 'A pair of massive, ornate doors lead to the throne room.');
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (12, 11, 4, 'north', 'A pair of massive, ornate doors lead back to the grand hall.');
-- Archives <-> Whispering Gallery
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (13, 2, 12, 'west', 'A narrow, curved hallway disappears to the west.');
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (14, 12, 2, 'east', 'The curved hallway opens back into the archives.');
-- Whispering Gallery <-> Alchemist's Laboratory
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (15, 12, 9, 'north', 'A door sealed with strange runes leads north.');
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (16, 9, 12, 'south', 'A rune-sealed door leads back to the gallery.');
-- Armory <-> Torture Chamber
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (17, 8, 6, 'down', 'A dark, foreboding passage slopes down.');
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (18, 6, 8, 'up', 'A dark passage slopes up towards the armory.');
-- Throne Room <-> Observatory
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (19, 11, 7, 'up', 'A hidden spiral staircase ascends upwards.');
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (20, 7, 11, 'down', 'A spiral staircase descends back to the throne room.');
-- Alchemist's Laboratory <-> Flooded Cavern
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (21, 9, 10, 'east', 'A crack in the wall leads to a damp cavern.');
INSERT INTO maps (map_id, from_room_id, to_room_id, direction, description) VALUES (22, 10, 9, 'west', 'A narrow crack in the wall leads back to the lab.');

--
-- Assumed Schema for Game Objects
--
-- Collection: game_objects
-- Description: Stores items that can be found in rooms.
-- Fields:
--   - object_id (NUMBER, PRIMARY KEY)
--   - name (STRING)
--   - description (STRING)
--   - location_room_id (NUMBER, FOREIGN KEY to rooms.room_id)
--   - is_takeable (BOOLEAN)
--

--
-- Insert data into the 'game_objects' collection
--
INSERT INTO game_objects (object_id, name, description, location_room_id, is_takeable) VALUES (1, 'Ancient Tome', 'A massive, leather-bound book with a tarnished silver clasp. Its pages are brittle, and the script is in a language you don''t recognize.', 1, FALSE);
INSERT INTO game_objects (object_id, name, description, location_room_id, is_takeable) VALUES (2, 'Obsidian Key', 'A small, intricately carved key made of pure obsidian. It feels cold to the touch.', 3, TRUE);
INSERT INTO game_objects (object_id, name, description, location_room_id, is_takeable) VALUES (3, 'Rusty Iron Pot', 'A heavy iron pot, large enough to cook a meal for a dozen people. It''s covered in rust and a thick layer of grime.', 5, FALSE);
INSERT INTO game_objects (object_id, name, description, location_room_id, is_takeable) VALUES (4, 'Skull', 'A human skull, yellowed with age, resting on a dusty shelf. Its empty eye sockets seem to watch you.', 6, TRUE);
INSERT INTO game_objects (object_id, name, description, location_room_id, is_takeable) VALUES (5, 'Brass Telescope Lens', 'A heavy, perfectly ground lens from the grand telescope. It seems to have fallen out of its housing.', 7, TRUE);
INSERT INTO game_objects (object_id, name, description, location_room_id, is_takeable) VALUES (6, 'Rusted Sword', 'A simple longsword, its blade heavily pitted with rust. It''s still surprisingly solid.', 8, TRUE);
INSERT INTO game_objects (object_id, name, description, location_room_id, is_takeable) VALUES (7, 'Unlabeled Potion', 'A small, corked bottle containing a swirling, pearlescent liquid. It gives off a faint warmth.', 9, TRUE);
INSERT INTO game_objects (object_id, name, description, location_room_id, is_takeable) VALUES (8, 'Glowing Mushroom', 'A fist-sized mushroom that pulses with a soft, ethereal blue light. It''s strangely beautiful.', 10, TRUE);
INSERT INTO game_objects (object_id, name, description, location_room_id, is_takeable) VALUES (9, 'Jeweled Scepter', 'A golden scepter lying on the steps of the dais, topped with a large, uncut ruby. It seems to have been dropped.', 11, TRUE);
INSERT INTO game_objects (object_id, name, description, location_room_id, is_takeable) VALUES (10, 'Faded Tapestry', 'A huge tapestry depicting a great battle. The colors are faded, and parts of it have rotted away, but the scene of carnage is still clear.', 4, FALSE);
