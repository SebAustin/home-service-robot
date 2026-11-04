#!/bin/bash
# Saves the map gmapping is building to catkin_ws/src/map/home_map.{pgm,yaml}.
# Run while test_slam.sh is still running.
source "$(dirname "$0")/common.sh"
require_built
source_workspace
rosrun map_server map_saver -f "$MAP_DIR/home_map" || fail "map_saver failed (is gmapping running?)"
# map_saver writes an absolute image path; make it relative so the repo works from any clone.
sed -i 's#^image: .*/home_map.pgm#image: home_map.pgm#' "$MAP_FILE"
echo "Saved $MAP_FILE"
