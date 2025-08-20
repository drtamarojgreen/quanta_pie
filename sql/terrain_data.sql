-- Terrain data for the game world, stored in tissdb.
-- tissdb is a tile-based storage system.

CREATE TABLE terrain (
    x_coord INT NOT NULL,
    y_coord INT NOT NULL,
    tile_type CHAR(1) NOT NULL, -- '#' for wall, '.' for empty space
    PRIMARY KEY (x_coord, y_coord)
);

-- Populate the map with some data.
-- This represents a small room.
-- #######
-- #.....#
-- #..p..#
-- #.....#
-- #######

INSERT INTO terrain (x_coord, y_coord, tile_type) VALUES
-- Walls
(0, 0, '#'), (1, 0, '#'), (2, 0, '#'), (3, 0, '#'), (4, 0, '#'), (5, 0, '#'), (6, 0, '#'),
(0, 1, '#'),                                                                   (6, 1, '#'),
(0, 2, '#'),                                                                   (6, 2, '#'),
(0, 3, '#'),                                                                   (6, 3, '#'),
(0, 4, '#'), (1, 4, '#'), (2, 4, '#'), (3, 4, '#'), (4, 4, '#'), (5, 4, '#'), (6, 4, '#'),

-- Floor
(1, 1, '.'), (2, 1, '.'), (3, 1, '.'), (4, 1, '.'), (5, 1, '.'),
(1, 2, '.'), (2, 2, '.'), (3, 2, '.'), (4, 2, '.'), (5, 2, '.'),
(1, 3, '.'), (2, 3, '.'), (3, 3, '.'), (4, 3, '.'), (5, 3, '.');
