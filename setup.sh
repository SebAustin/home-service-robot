#!/bin/bash
# One-time setup on Ubuntu 20.04 + ROS Noetic (the Udacity/Vocareum workspace), then build.
# Usage: ./setup.sh            (from the repository root)
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"

sudo apt-get update
sudo apt-get install -y \
  xterm \
  ros-noetic-navigation \
  ros-noetic-openslam-gmapping

# Vendored sources (slam_gmapping, teleop_twist_keyboard) are already in catkin_ws/src;
# see catkin_ws/src/VENDORED_SOURCES.txt for their upstream commits.
set +u  # ROS setup scripts read unset variables
source /opt/ros/noetic/setup.bash
cd "$ROOT/catkin_ws"
rosdep install --from-paths src --ignore-src -r -y || echo "rosdep reported missing keys (see above)"
catkin_make
catkin_make run_tests && catkin_test_results build/test_results

echo
echo "Built. Next: catkin_ws/src/scripts/test_slam.sh"
