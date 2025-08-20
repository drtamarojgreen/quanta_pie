-- SQL script for TissDB game data.
-- This script has been refactored to represent a journey of Cognitive Behavioral Therapy (CBT).

-- =============================================================================
-- Mindscape Data (Game World)
-- =============================================================================

--
-- Collection: rooms
-- Description: Stores information about each room in the mindscape.
-- Fields:
--   - room_id (NUMBER, PRIMARY KEY)
--   - description (STRING)
--   - ascii_art (STRING)
--
INSERT INTO rooms (room_id, description, ascii_art) VALUES (1, 'You are in the Library of Core Beliefs. Ancient, sturdy shelves hold the foundational ideas that shape your world. Some books glow with a warm, steady light, while others are heavy and bound in shadow. This is where your fundamental understanding of yourself and the world resides.', '   _______
  /      /,
 /      //
/______//
(______(/');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (2, 'You have entered the Archives of Automatic Thoughts. Here, fleeting thoughts and immediate reactions are recorded on endless scrolls that zip by on brass tracks. Most are mundane, but some are inscribed with heavily biased or critical ink.', '||||||||
|      |
|      |
||||||||');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (3, 'This is the Quiet Nook of Mindfulness. A comfortable chair sits before a calm, steady hearth. The air is still and quiet. This is a space for non-judgmental observation of your thoughts and feelings as they are.', '  )  (
 /`--`\
|      |
|______|');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (4, 'You are in the Grand Hall of Social Interactions. Echoes of past conversations hang in the air. The space is vast and can feel either welcoming or intimidating depending on the light, which shifts with your mood.', '   | |         | |
|  | | |     | | |  |
|  | | |     | | |  |
|__| |_|_____|_| |__| 
(____________________)');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (5, 'This is the Kitchen of Physical Sensations. The smells of different foods and the feeling of warmth from the stove connect you to your body. This room reminds you of the link between physical well-being and emotional states.', ',-----------.
|]         [|
|           |
|___________|
|o|       |o|
|=|       |=|');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (6, 'You''ve entered the Room of Cognitive Distortions. The walls seem to twist and shimmer, presenting funhouse-mirror reflections of reality. Whispers of all-or-nothing thinking, overgeneralization, and catastrophizing echo from the corners. This room is uncomfortable, but understanding it is key.', '/`-.--`-.
| o| o |
`-/`--`\
  |     |
 /|     |\
/ |     | \
`=` `---` =`');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (7, 'You are in the Observatory of Perspective. A large, clear lens allows you to look out at your own thoughts and situations from a detached, higher vantage point. From here, problems that seemed immense can look manageable.', '   .---.
  /     \
 |       |
 |   o   |
/|/\   /|\
( |  `=`  | )
`|_______|`');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (8, 'This is the Workshop of Coping Strategies. Workbenches are covered with tools for managing difficult emotions and situations. Here you can find plans for deep breathing, grounding exercises, and problem-solving.', '|` |` |` |
|-`|-`|-`|
|  |  |  |
(-----)  |
| | | |  |
|_|_|_|__|');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (9, 'You are in the Laboratory of Behavioral Experiments. This room is for testing beliefs and assumptions by taking action. It''s a safe space to try new things and see if the predicted negative outcomes actually occur.', '  .---.
 (     )
  `---`
 /  |  \
(   |   )
 `--`--`');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (10, 'You have found the Flooded Cavern of Emotions. A deep, calm pool of water reflects the glowing moss on the ceiling. This room represents the full spectrum of your emotions. The water can be calm or turbulent, but it is always just water.', ',^,^,^,^,
/         \
(           )
~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~
(___________)');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (11, 'This is the Throne Room of the Self. A simple, comfortable chair sits on a modest platform. This is the seat of your executive function, where you make conscious choices and decisions. It is a place of self-acceptance and agency.', '  |---| 
 /     \
|  _|_  |
| (___) |
|_______|');
INSERT INTO rooms (room_id, description, ascii_art) VALUES (12, 'You are in the Whispering Gallery of Others'' Perspectives. Faint whispers move along the curved walls, allowing you to hear empathetic echoes of how others might feel or think in a given situation. It is a place to practice empathy.', '/`--------`\
/           \
/             \
(               )
(               )
\             /
 \           /
  `---------`');

--
-- Collection: room_objects
-- Description: Stores static, decorative scenery items in rooms.
-- Fields:
--   - object_id (NUMBER, PRIMARY KEY)
--   - name (STRING)
--   - description (STRING)
--   - room_id (NUMBER, FOREIGN KEY to rooms.room_id)
--
INSERT INTO room_objects (object_id, name, description, room_id) VALUES (1, 'Book of Core Beliefs', 'A large, heavy book lying open on a lectern. Its pages describe a fundamental belief, such as "I must be perfect to be worthy."', 1);
INSERT INTO room_objects (object_id, name, description, room_id) VALUES (2, 'Pot of Nourishing Soup', 'A pot of warm, fragrant soup on the stove, a reminder to care for your physical needs.', 5);
INSERT INTO room_objects (object_id, name, description, room_id) VALUES (3, 'Tapestry of a Past Success', 'A tapestry depicting a time you successfully navigated a difficult social situation. The colors are vibrant and clear.', 4);

--
-- Collection: tools
-- Description: Stores takeable, interactive items representing CBT techniques.
-- Fields:
--   - tool_id (NUMBER, PRIMARY KEY)
--   - name (STRING)
--   - description (STRING)
--   - initial_room_id (NUMBER, FOREIGN KEY to rooms.room_id)
--
INSERT INTO tools (tool_id, name, description, initial_room_id) VALUES (1, 'Key of Awareness', 'A simple, unadorned key. It doesn''t unlock doors, but rather the awareness of what is happening in the present moment.', 3);
INSERT INTO tools (tool_id, name, description, initial_room_id) VALUES (2, 'Thought Record', 'A small, empty journal page. It seems to be for recording a situation, the automatic thoughts that followed, and a more balanced, alternative thought.', 6);
INSERT INTO tools (tool_id, name, description, initial_room_id) VALUES (3, 'Lens of Reframing', 'A beautifully crafted lens. Looking through it allows you to see a situation from a different, more helpful perspective.', 7);
INSERT INTO tools (tool_id, name, description, initial_room_id) VALUES (4, 'Blueprint for Grounding', 'A simple diagram illustrating a 5-4-3-2-1 grounding exercise. It seems to be a reliable plan for managing overwhelming feelings.', 8);
INSERT INTO tools (tool_id, name, description, initial_room_id) VALUES (5, 'Vial of Courage', 'A small bottle containing a swirling, golden liquid. It is not a magical potion, but a reminder of your own inner strength to face a fear.', 9);
INSERT INTO tools (tool_id, name, description, initial_room_id) VALUES (6, 'Smooth Stone of Mindfulness', 'A perfectly smooth, cool stone that fits comfortably in your palm. Holding it helps anchor you to the present moment.', 10);
INSERT INTO tools (tool_id, name, description, initial_room_id) VALUES (7, 'Scepter of Self-Compassion', 'A simple, warm wooden rod. Holding it reminds you to treat yourself with the same kindness you would offer a friend.', 11);
INSERT INTO tools (tool_id, name, description, initial_room_id) VALUES (8, 'Key of Values', 'A key etched with your most important personal values. It unlocks the door to the Workshop of Coping Strategies, reminding you that your values guide your actions.', 4);

--
-- Collection: exits
-- Description: Stores detailed connections (exits) between rooms.
-- Fields:
--   - exit_id (NUMBER, PRIMARY KEY)
--   - from_room_id (NUMBER, FOREIGN KEY to rooms.room_id)
--   - to_room_id (NUMBER, FOREIGN KEY to rooms.room_id)
--   - direction (STRING)
--   - description (STRING)
--   - is_locked (BOOLEAN)
--   - key_tool_id (NUMBER, FOREIGN KEY to tools.tool_id, NULLABLE)
--
-- Connections are mostly the same, but the descriptions are updated.
-- Library of Core Beliefs <-> Archives of Automatic Thoughts
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (1, 1, 2, 'north', 'An archway leads to the bustling archives.', FALSE, NULL);
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (2, 2, 1, 'south', 'An archway leads back to the quiet library.', FALSE, NULL);
-- Library of Core Beliefs <-> Quiet Nook of Mindfulness
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (3, 1, 3, 'east', 'A quiet curtain conceals a passage to a place of mindfulness.', FALSE, NULL);
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (4, 3, 1, 'west', 'A quiet curtain conceals a passage back to the library.', FALSE, NULL);
-- Library of Core Beliefs <-> Grand Hall of Social Interactions
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (5, 1, 4, 'down', 'A grand staircase descends into the hall of social interactions.', FALSE, NULL);
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (6, 4, 1, 'up', 'A grand staircase ascends to the library of core beliefs.', FALSE, NULL);
-- Grand Hall of Social Interactions <-> Kitchen of Physical Sensations
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (7, 4, 5, 'east', 'A swinging door leads into the kitchen of physical sensations.', FALSE, NULL);
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (8, 5, 4, 'west', 'A swinging door leads back to the grand hall.', FALSE, NULL);
-- Grand Hall of Social Interactions <-> Workshop of Coping Strategies
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (9, 4, 8, 'north', 'A sturdy door opens into the workshop of coping strategies.', FALSE, NULL);
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (10, 8, 4, 'south', 'A sturdy door opens back into the grand hall.', FALSE, NULL);
-- Grand Hall of Social Interactions <-> Throne Room of the Self
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (11, 4, 11, 'south', 'A pair of simple, elegant doors lead to the throne room of the self.', FALSE, NULL);
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (12, 11, 4, 'north', 'A pair of simple, elegant doors lead back to the grand hall.', FALSE, NULL);
-- Archives of Automatic Thoughts <-> Whispering Gallery of Others'' Perspectives
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (13, 2, 12, 'west', 'A narrow, curved hallway disappears to the west.', FALSE, NULL);
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (14, 12, 2, 'east', 'The curved hallway opens back into the archives.', FALSE, NULL);
-- Whispering Gallery <-> Laboratory of Behavioral Experiments (LOCKED)
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (15, 12, 9, 'north', 'A door that requires the Key of Awareness to open.', TRUE, 1);
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (16, 9, 12, 'south', 'A door sealed by a lack of awareness.', TRUE, 1);
-- Workshop of Coping Strategies <-> Room of Cognitive Distortions (LOCKED)
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (17, 8, 6, 'down', 'A heavy door, locked by avoidance. It requires the Key of Values to proceed.', TRUE, 8);
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (18, 6, 8, 'up', 'The heavy door is locked from this side, requiring you to align with your values.', TRUE, 8);
-- Throne Room of the Self <-> Observatory of Perspective
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (19, 11, 7, 'up', 'A hidden spiral staircase ascends towards a new perspective.', FALSE, NULL);
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (20, 7, 11, 'down', 'A spiral staircase descends back to the seat of self.', FALSE, NULL);
-- Laboratory of Behavioral Experiments <-> Flooded Cavern of Emotions
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (21, 9, 10, 'east', 'A crack in the wall leads to the cavern of emotions.', FALSE, NULL);
INSERT INTO exits (exit_id, from_room_id, to_room_id, direction, description, is_locked, key_tool_id) VALUES (22, 10, 9, 'west', 'A narrow crack in the wall leads back to the lab.', FALSE, NULL);


--
-- Collection: characters
-- Description: Stores non-player characters (NPCs) in the game world.
-- Fields:
--   - character_id (NUMBER, PRIMARY KEY)
--   - name (STRING)
--   - description (STRING)
--   - initial_room_id (NUMBER, FOREIGN KEY to rooms.room_id)
--   - dialogue (STRING)
--
INSERT INTO characters (character_id, name, description, initial_room_id, dialogue) VALUES (1, 'The Guide', 'A calm and gentle figure stands here, radiating a quiet confidence. They offer a warm smile.', 11, 'What is one piece of evidence that supports your thought? And what is one piece that contradicts it?');
INSERT INTO characters (character_id, name, description, initial_room_id, dialogue) VALUES (2, 'The Echo of Doubt', 'A flickering, shadowy figure darts around the edges of the room, muttering in a critical tone. It seems to be made of shifting doubt.', 6, 'You''ll probably fail, so why even try? You always mess things up.');

-- =============================================================================
-- Player and Session Data (Unchanged for this refactoring)
-- =============================================================================

--
-- Collection: players
--
INSERT INTO players (player_id, name, join_date) VALUES (1, 'Alice', '2023-01-15 10:30:00');
INSERT INTO players (player_id, name, join_date) VALUES (2, 'Bob', '2023-01-16 11:00:00');
INSERT INTO players (player_id, name, join_date) VALUES (3, 'Charlie', '2023-01-17 12:15:00');

--
-- Collection: game_sessions
--
INSERT INTO game_sessions (session_id, game_type, start_time, end_time) VALUES (101, 'Chess', '2023-02-01 14:00:00', '2023-02-01 15:00:00');
INSERT INTO game_sessions (session_id, game_type, start_time, end_time) VALUES (102, 'Checkers', '2023-02-02 16:00:00', '2023-02-02 16:30:00');
INSERT INTO game_sessions (session_id, game_type, start_time, end_time) VALUES (103, 'Chess', '2023-02-03 18:00:00', '2023-02-03 19:30:00');

--
-- Collection: scores
--
INSERT INTO scores (score_id, player_id, session_id, score) VALUES (1001, 1, 101, 10);
INSERT INTO scores (score_id, player_id, session_id, score) VALUES (1002, 2, 101, 5);
INSERT INTO scores (score_id, player_id, session_id, score) VALUES (1003, 2, 102, 20);
INSERT INTO scores (score_id, player_id, session_id, score) VALUES (1004, 3, 102, 15);
INSERT INTO scores (score_id, player_id, session_id, score) VALUES (1005, 1, 103, 12);
INSERT INTO scores (score_id, player_id, session_id, score) VALUES (1006, 3, 103, 8);