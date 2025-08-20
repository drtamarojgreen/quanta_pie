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
