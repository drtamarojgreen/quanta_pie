-- SQL script for TissDB game data.
-- This script is no longer used to populate the game world with data.
-- All game data is now loaded from the CSV files in this directory.
-- This file is kept for historical purposes and to show the table schema.

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

--
-- Collection: room_objects
-- Description: Stores static, decorative scenery items in rooms.
-- Fields:
--   - object_id (NUMBER, PRIMARY KEY)
--   - name (STRING)
--   - description (STRING)
--   - room_id (NUMBER, FOREIGN KEY to rooms.room_id)
--

--
-- Collection: tools
-- Description: Stores takeable, interactive items representing CBT techniques.
-- Fields:
--   - tool_id (NUMBER, PRIMARY KEY)
--   - name (STRING)
--   - description (STRING)
--   - initial_room_id (NUMBER, FOREIGN KEY to rooms.room_id)
--

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

-- =============================================================================
-- Player and Session Data
-- =============================================================================

--
-- Collection: players
--

--
-- Collection: game_sessions
--

--
-- Collection: scores
--
