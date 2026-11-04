# Use my_robot on ROS Noetic instead of the course's TurtleBot 2

The course instructions assume ROS Kinetic and the TurtleBot 2 packages (`turtlebot_gazebo`, `turtlebot_teleop`, `turtlebot_rviz_launchers`), but the Udacity workspace runs Ubuntu 20.04 with ROS Noetic, where TurtleBot 2 is not packaged. We reuse `my_robot` from the "Where Am I?" project, which already navigates `sebastien.world` on Noetic with a tuned AMCL + move_base setup and carries a real 2D lidar that suits gmapping better than TurtleBot 2's depth-camera fake laser.

## Considered Options

- **TurtleBot 2 from source on Noetic**: the Kinetic-era repos (kobuki, turtlebot_simulator) do not build without porting work.
- **TurtleBot3** (packaged for Noetic): viable, but navigation would have to be re-tuned for this world from scratch, and the rubric explicitly invites a personalized robot.

## Consequences

- The scripts launch `my_robot` launch files (`world.launch`, `gmapping.launch`, `amcl.launch`) and `teleop_twist_keyboard` in place of the TurtleBot equivalents named in the course text.
- Only `slam_gmapping` and `teleop_twist_keyboard` are vendored; the navigation stack comes from apt (`setup.sh`).
