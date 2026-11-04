#!/bin/bash
# Manual navigation test: robot localized with AMCL on the saved map, move_base ready.
# In RViz use "2D Nav Goal" to send the robot anywhere.
source "$(dirname "$0")/common.sh"
require_built
require_map

launch_world
launch_navigation
launch_rviz
echo "Send goals with the 2D Nav Goal tool in RViz."
