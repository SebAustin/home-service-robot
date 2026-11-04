#!/bin/bash
# Virtual Object on a timer, no robot motion: shown at the Pickup Zone, hidden after 5 s,
# shown at the Drop-off Zone 5 s later.
source "$(dirname "$0")/common.sh"
require_built
require_map

launch_world
load_zones
launch_navigation
launch_rviz
in_xterm "add_markers (timed)" rosrun add_markers add_markers _mode:=timed
