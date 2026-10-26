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
-- Gauntlet Data (the 100-room Emotional Intelligence and Resilience game)
-- =============================================================================

--
-- Collection: gauntlet_rooms
-- Description: The rooms, in order. Room ids must run 1, 2, 3... without gaps.
-- Fields:
--   - room_id (NUMBER, PRIMARY KEY)
--   - name (STRING)
--   - description (STRING)
--   - layout_id (NUMBER, FOREIGN KEY to gauntlet_layouts.layout_id)
--

--
-- Collection: gauntlet_room_obstacles
-- Description: The obstacles in each room, cleared in sequence order. The last one
--              stands on the door; the others stand on the layout's obstacle points.
--              The shipped rooms grow harder one by one: room n has ceil(sqrt(n))
--              obstacles needing n EI and R in total.
-- Fields:
--   - room_obstacle_id (NUMBER, PRIMARY KEY)
--   - room_id (NUMBER, FOREIGN KEY to gauntlet_rooms.room_id)
--   - sequence (NUMBER, 1 and up, no gaps within a room)
--   - obstacle_type (NUMBER, FOREIGN KEY to gauntlet_obstacles.obstacle_type, 1-10)
--

--
-- Collection: gauntlet_layouts
-- Description: A room's tile grid. The obstacle stands on the north door tile;
--              the player arrives from the previous room on the south entry tile.
-- Fields:
--   - layout_id (NUMBER, PRIMARY KEY)
--   - name (STRING)
--   - width (NUMBER)
--   - height (NUMBER)
--   - door_x, door_y (NUMBER)
--   - entry_x, entry_y (NUMBER)
--

--
-- Collection: gauntlet_spawn_points
-- Description: The fixed tiles where equipment spawns. They hold the equipment for
--              the room's next obstacle: one needing N EI fills EI slots 1..N of
--              the layout, and likewise for R. They refill as each obstacle is cleared.
-- Fields:
--   - spawn_point_id (NUMBER, PRIMARY KEY)
--   - layout_id (NUMBER, FOREIGN KEY to gauntlet_layouts.layout_id)
--   - kind (STRING, 'EI' or 'R')
--   - slot (NUMBER, 1 and up)
--   - x, y (NUMBER)
--

--
-- Collection: gauntlet_obstacle_points
-- Description: Where a layout places a room's obstacles, except the last (on the door).
--              Obstacle 1 stands on slot 1, obstacle 2 on slot 2, and so on.
-- Fields:
--   - obstacle_point_id (NUMBER, PRIMARY KEY)
--   - layout_id (NUMBER, FOREIGN KEY to gauntlet_layouts.layout_id)
--   - slot (NUMBER, 1 and up)
--   - x, y (NUMBER)
--

--
-- Collection: gauntlet_hazards
-- Description: Fixed tiles in a layout that cost extra health to step on.
-- Fields:
--   - hazard_id (NUMBER, PRIMARY KEY)
--   - layout_id (NUMBER, FOREIGN KEY to gauntlet_layouts.layout_id)
--   - x, y (NUMBER; not on the door, the entry, a spawn point or an obstacle point)
--   - damage (NUMBER, 1 and up)
--   - name (STRING)
--   - description (STRING)
--

--
-- Collection: gauntlet_exits
-- Description: Connections between gauntlet rooms.
-- Fields:
--   - exit_id (NUMBER, PRIMARY KEY)
--   - from_room_id (NUMBER, FOREIGN KEY to gauntlet_rooms.room_id)
--   - to_room_id (NUMBER, FOREIGN KEY to gauntlet_rooms.room_id)
--   - direction (STRING, 'north' through the door or 'south' through the entry)
--   - description (STRING)
--

--
-- Collection: gauntlet_obstacles
-- Description: The ten obstacle types. Types 1-5 need 1-5 EI; types 6-10 need
--              5 EI plus 1-5 R (the requirements are rules of the game, not data).
-- Fields:
--   - obstacle_type (NUMBER, PRIMARY KEY, 1-10)
--   - symbol (STRING, one character drawn on the map; unique, and none of . @ E R # + - | ^ v :)
--   - name (STRING)
--   - description (STRING)
--

--
-- Collection: gauntlet_equipment
-- Description: The equipment that spawns in each EI or R slot.
-- Fields:
--   - equipment_id (NUMBER, PRIMARY KEY)
--   - kind (STRING, 'EI' or 'R')
--   - slot (NUMBER)
--   - name (STRING)
--   - description (STRING)
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
