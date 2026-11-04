#!/bin/bash
# Course warm-up: Gazebo, the ROS master and RViz, each in its own xterm.
# (bash, not sh: Ubuntu's /bin/sh is dash, which has no `source`.)
xterm -hold -e "gazebo" &
sleep 5
xterm -hold -e "source /opt/ros/noetic/setup.bash; roscore" &
sleep 5
xterm -hold -e "source /opt/ros/noetic/setup.bash; rosrun rviz rviz" &
