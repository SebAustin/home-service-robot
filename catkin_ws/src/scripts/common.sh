#!/bin/bash
# Shared helpers for the Home Service Robot scripts. Source this file; do not run it.
#
# Every node runs in its own xterm so each one's output can be followed separately.
# `xterm -hold` keeps a window open after its node exits, so final messages stay readable.

SCRIPTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$(dirname "$SCRIPTS_DIR")"
WS_DIR="$(dirname "$SRC_DIR")"

MAP_DIR="$SRC_DIR/map"
WORLD_FILE="$MAP_DIR/home.world"
MAP_FILE="$MAP_DIR/home_map.yaml"
ZONES_FILE="$MAP_DIR/zones.yaml"
RVIZ_CONFIG="$SRC_DIR/rvizConfig/home_service.rviz"

ROS_SETUP="/opt/ros/noetic/setup.bash"
WS_SETUP="$WS_DIR/devel/setup.bash"

# Seconds to let each node come up before starting the next one.
STEP_DELAY="${STEP_DELAY:-5}"

fail() {
  echo "ERROR: $*" >&2
  exit 1
}

require_built() {
  command -v xterm >/dev/null || fail "xterm is not installed (run setup.sh)"
  [ -f "$ROS_SETUP" ] || fail "ROS Noetic not found at $ROS_SETUP"
  [ -f "$WS_SETUP" ] || fail "workspace not built: cd \"$WS_DIR\" && catkin_make"
}

require_map() {
  [ -f "$MAP_FILE" ] || fail "no map at $MAP_FILE: run test_slam.sh, drive around, then save_map.sh"
}

source_workspace() {
  # shellcheck disable=SC1090
  source "$ROS_SETUP" && source "$WS_SETUP"
}

# in_xterm "Window title" command args...
# Runs the command in a new xterm with the workspace sourced, then waits STEP_DELAY seconds.
in_xterm() {
  local title="$1"
  shift
  local cmd
  printf -v cmd '%q ' "$@"
  xterm -hold -T "$title" -e bash -c \
    "source $(printf %q "$ROS_SETUP") && source $(printf %q "$WS_SETUP") && $cmd" &
  sleep "$STEP_DELAY"
}

# Blocks until the ROS master answers (Gazebo's roslaunch starts it).
wait_for_master() {
  source_workspace
  echo "Waiting for the ROS master..."
  until rostopic list >/dev/null 2>&1; do sleep 1; done
}

load_zones() {
  rosparam load "$ZONES_FILE" || fail "could not load $ZONES_FILE"
  echo "Zones loaded from $ZONES_FILE"
}

launch_world() {
  in_xterm "Gazebo world" roslaunch my_robot world.launch world_file:="$WORLD_FILE"
  wait_for_master
}

launch_navigation() {
  in_xterm "AMCL + move_base" roslaunch my_robot amcl.launch map_file:="$MAP_FILE"
}

launch_rviz() {
  in_xterm "RViz" rosrun rviz rviz -d "$RVIZ_CONFIG"
}
