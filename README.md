# Home Service Robot

Udacity Robotics Software Engineer, final project. A simulated robot maps a two-room home with SLAM, localizes in that map, and autonomously carries a **Virtual Object** from a **Pickup Zone** in the west room to a **Drop-off Zone** in the east room, with the object drawn in RViz. Terms are defined in [GLOSSARY.md](GLOSSARY.md); key decisions are in [docs/adr](docs/adr).

Target: **Ubuntu 20.04 + ROS Noetic** (the Udacity workspace). The course text assumes Kinetic and TurtleBot 2, which Noetic does not package; this project uses its own robot instead ([ADR 0002](docs/adr/0002-own-robot-on-noetic.md)).

## Packages used

### Mapping: `gmapping` (SLAM)
`slam_gmapping` is a Rao-Blackwellized particle filter: each particle carries a robot trajectory hypothesis and its own occupancy grid. It fuses the robot's 2D lidar (`/scan`) with wheel odometry to build the map and the robot's pose at the same time, publishing `map → odom`. `my_robot/launch/gmapping.launch` sizes the grid for the 8 × 6 m home at 5 cm cells and assumes low odometry noise (Gazebo odometry is near ground truth). The robot drives under keyboard control from `teleop_twist_keyboard`; `map_server`'s `map_saver` writes the result to `map/home_map.{pgm,yaml}`.

### Localization: `amcl`
Adaptive Monte Carlo Localization tracks the robot on the **saved** map with a particle filter. Each particle is a pose guess; the lidar scan reweights particles by how well it matches the map (likelihood-field model), and resampling keeps the plausible ones. KLD sampling adapts the particle count (500–2500) to how certain the filter is. Initial pose is the spawn pose (0, 0, 0).

### Navigation: `move_base`
- **Global planner** `navfn`: Dijkstra's algorithm (a uniform-cost search) over the global costmap, from the robot to the goal.
- **Local planner** `base_local_planner/TrajectoryPlannerROS`: simulates short velocity commands and picks the one that follows the global plan while avoiding obstacles the lidar sees, re-planning around new obstacles.
- **Costmaps**: obstacles from the map and the lidar, inflated by 0.35 m around the robot's real 0.5 × 0.3 m footprint.

`move_base` decides **Arrival**: a goal counts as reached when it reports `SUCCEEDED` (within 0.15 m and 0.15 rad).

### Gazebo / robot: `my_robot`
Differential-drive robot with a Hokuyo lidar and a camera, from the earlier "Where Am I?" project. `world.launch` spawns it at the world origin, so gmapping's map frame equals the Gazebo frame and zone coordinates can be read off the world file.

### Custom packages

| Package | What it does |
|---|---|
| `pick_objects` | Sends the Pickup Zone, then the Drop-off Zone, to `move_base` as actionlib goals in the `map` frame. Prints `Reached pickup zone`, waits 5 s, then `Reached drop-off zone`. Retries a failed goal up to 3 times per zone. Announces its **Delivery Phase** on a latched `/delivery_status` topic: `HEADING_TO_PICKUP → AT_PICKUP → HEADING_TO_DROPOFF → DELIVERED` (or `FAILED`). |
| `add_markers` | Draws the Virtual Object as an amber 0.25 m cube on `/visualization_marker`. `_mode:=timed`: pickup 5 s → hidden 5 s → drop-off. `_mode:=reactive`: follows `/delivery_status` (hidden on `AT_PICKUP`, shown at the drop-off on `DELIVERED`). Re-sends its state at 2 Hz so a late-starting RViz still sees it. |

**Why a status topic and not odometry?** The course suggests `add_markers` subscribe to `/odom` and check the distance to each zone. Odometry is in the drifting `odom` frame, not the `map` frame the zones are in, and a second distance threshold could disagree with move_base. `pick_objects` is the only node that knows the goal succeeded, so it announces it ([ADR 0001](docs/adr/0001-delivery-status-topic.md)).

Zones live in one file, [`map/zones.yaml`](catkin_ws/src/map/zones.yaml), loaded as ROS parameters and read by both nodes through the same function.

## Layout

```
catkin_ws/src/
├── slam_gmapping/          vendored (see VENDORED_SOURCES.txt)
├── teleop_twist_keyboard/  vendored
├── my_robot/               robot URDF, world/gmapping/amcl launch files, navigation config
├── map/                    home.world, zones.yaml, home_map.{pgm,yaml} (from SLAM)
├── scripts/                shell scripts (below) + common.sh helpers
├── rvizConfig/             home_service.rviz
├── pick_objects/           pick_objects node + Delivery state machine
└── add_markers/            add_markers node + marker placement rules
```

## Build

```bash
./setup.sh
```

Installs `xterm`, the Noetic navigation stack and `openslam_gmapping`, then runs `catkin_make` and the unit tests.

If `apt-get update` fails with `EXPKEYSIG F42ED6FBAB17C654`, the workspace's ROS signing key has expired (`setup.sh` detects this and stops). Refresh it, then rerun `./setup.sh`:

```bash
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg
echo 'deb [signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros/ubuntu focal main' | sudo tee /etc/apt/sources.list.d/ros-latest.list
```

## Run

Each script opens one xterm per node, 5 s apart.

| Script | Starts | What to check |
|---|---|---|
| `launch.sh` | Gazebo, roscore, RViz | Warm-up from the course |
| `test_slam.sh` | world, gmapping, RViz, teleop | Click the Teleop xterm, drive both rooms; map grows in RViz. Then run `save_map.sh`. |
| `test_navigation.sh` | world, AMCL + move_base, RViz | Particle cloud around the robot; "2D Nav Goal" drives it there |
| `pick_objects.sh` | + `pick_objects` | Robot drives to pickup, `Reached pickup zone`, 5 s pause, drives to drop-off, `Reached drop-off zone` |
| `add_marker.sh` | + `add_markers` (timed) | Cube at pickup → disappears after 5 s → appears at drop-off 5 s later |
| `home_service.sh` | + `add_markers` (reactive) + `pick_objects` | Cube at pickup; disappears when the robot arrives; robot waits 5 s, drives to drop-off; cube appears there |

All scripts live in `catkin_ws/src/scripts/`. Run `stop_all.sh` between scripts to close the previous simulation. Every script except `test_slam.sh` needs the saved map.

## Tests

`catkin_make run_tests` runs 24 gtest cases on the logic with no ROS needed: zone validation, Delivery Phase names (the `/delivery_status` contract), the Delivery state machine (order, retries, invalid events), and the marker placement rules for both modes. The simulation itself is checked by hand with the table above.

## Verified run (Udacity workspace, Ubuntu 20.04 + Noetic)

- `setup.sh`: build OK, 78 tests, 0 failures (includes gmapping's own tests).
- `test_slam.sh`: map built by gmapping while driving both rooms; saved as `map/home_map.{pgm,yaml}`.
- `home_service.sh`: `HEADING_TO_PICKUP` → *Reached pickup zone* at 30.5 s (sim) and the Virtual Object hidden at the same instant → 5.0 s pickup → *Reached drop-off zone* at 55.7 s and the Virtual Object shown at the drop-off. Both legs on the first attempt.
- `add_marker.sh`: shown at pickup → hidden 5 s later → shown at drop-off 5 s after that.
