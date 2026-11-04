#!/bin/bash
# Full home service: the Virtual Object waits at the Pickup Zone, disappears when the robot
# arrives, and reappears at the Drop-off Zone when the robot delivers it.
source "$(dirname "$0")/common.sh"
require_built
require_map

launch_world
load_zones
launch_navigation
launch_rviz
in_xterm "add_markers (reactive)" rosrun add_markers add_markers _mode:=reactive
in_xterm "pick_objects" rosrun pick_objects pick_objects
