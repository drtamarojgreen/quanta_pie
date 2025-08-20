-- Test data for Quanta_Pie
-- This script sets up a minimal world for testing.

-- Rooms
-- 1: Starting Room
-- 2: Blue Room
-- 3: Locked Room
INSERT INTO rooms (id, description) VALUES (1, 'You are in a plain, white room. There is a door to the north.');
INSERT INTO rooms (id, description) VALUES (2, 'This room is entirely blue. There is a door to the south.');
INSERT INTO rooms (id, description) VALUES (3, 'This room is dark. The door is locked.');

-- Exits
-- Connects room 1 to room 2
INSERT INTO exits (from_room_id, to_room_id, direction) VALUES (1, 2, 'north');
-- Connects room 2 back to room 1
INSERT INTO exits (from_room_id, to_room_id, direction) VALUES (2, 1, 'south');

-- Characters
INSERT INTO characters (id, name, room_id) VALUES (1, 'Mysterious Figure', 2);

-- Game Sessions (optional, for session-related tests)
INSERT INTO game_sessions (id, player_id, start_time) VALUES (1, 1, '2023-10-27 10:00:00');

-- Players
INSERT INTO players (id, name, current_room_id) VALUES (1, 'Test Player', 1);
