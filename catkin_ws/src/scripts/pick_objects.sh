#!/bin/bash
# Autonomous Delivery without the Virtual Object: pick_objects drives to the Pickup Zone,
# waits 5 s, then drives to the Drop-off Zone, logging each arrival.
source "$(dirname "$0")/common.sh"
require_built
require_map

launch_world
load_zones
launch_navigation
launch_rviz
in_xterm "pick_objects" rosrun pick_objects pick_objects
