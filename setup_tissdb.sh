#!/bin/bash
# This script sets up the tissdb database.
# It clones the necessary repository, builds the database,
# and initializes it with terrain data.

# Exit immediately if a command exits with a non-zero status.
set -e

# 1. Clone the quanta_tissu repository
echo "--> Cloning the repository..."
if [ -d "quanta_tissu" ]; then
    echo "--> 'quanta_tissu' directory already exists. Skipping clone."
else
    git clone https://github.com/drtamarojgreen/quanta_tissu.git
fi

# 2. Build tissdb
echo "--> Building tissdb..."
cd quanta_tissu/tissdb
# We assume 'make' is the correct build command based on common C++ practices,
# as project documentation was not accessible.
make
cd ../.. # Return to project root

# 3. Run the SQL script to initialize the database
echo "--> Initializing the database..."
# The following command is an assumption on how to run the SQL script.
# It assumes the 'make' command creates an executable named 'tissdb'
# in a 'bin' directory inside 'quanta_tissu/tissdb' and that this
# executable can process the SQL file from standard input.
quanta_tissu/tissdb/bin/tissdb < sql/terrain_data.sql

echo "--> Setup script finished."
