#!/bin/bash
# One-time setup on Ubuntu 20.04 + ROS Noetic (the Udacity/Vocareum workspace), then build.
# Usage: ./setup.sh            (from the repository root)
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"

update_log="$(mktemp)"
if ! sudo apt-get update 2>&1 | tee "$update_log"; then
  echo "apt-get update failed" >&2
fi
if grep -q "EXPKEYSIG F42ED6FBAB17C654" "$update_log"; then
  cat >&2 <<'MSG'

The ROS apt signing key on this machine has expired. Refresh it, then run ./setup.sh again:

  sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg
  echo 'deb [signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros/ubuntu focal main' | sudo tee /etc/apt/sources.list.d/ros-latest.list

MSG
  exit 1
fi
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
catkin_make run_tests
catkin_test_results build/test_results  # non-zero (and setup fails) if any test failed

echo
echo "Built. Next: catkin_ws/src/scripts/test_slam.sh"
