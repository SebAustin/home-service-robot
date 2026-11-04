#!/bin/bash
# Manual SLAM test: robot in the home world, gmapping, RViz, and keyboard teleop.
# Drive with the keys shown in the "Teleop" xterm (click it first), then run save_map.sh.
source "$(dirname "$0")/common.sh"
require_built

launch_world
in_xterm "gmapping (SLAM)" roslaunch my_robot gmapping.launch
launch_rviz
in_xterm "Teleop (click here, then drive)" rosrun teleop_twist_keyboard teleop_twist_keyboard.py

echo "Mapping. Suggested route: east room loop, through the doorway, west room loop, back to start."
echo "When the map looks complete in RViz, run: $SCRIPTS_DIR/save_map.sh"
