#!/bin/bash
# Closes every window and node a previous script started (xterms, Gazebo, RViz, the ROS master),
# so the next script starts from a clean simulation.
killall -q xterm
killall -q gzserver gzclient rviz roslaunch rosmaster
sleep 2
echo "Stopped."
