# The robot announces its Delivery Phase; the marker node does not track position

`add_markers` learns when to hide and re-show the Virtual Object by listening to a latched `/delivery_status` topic published by `pick_objects`, not by subscribing to `/odom` (as the course suggests) or `/amcl_pose` and comparing positions to the zones. `pick_objects` is the only node that knows when `move_base` actually succeeded, so making it the single source of truth avoids two nodes disagreeing about Arrival.

## Considered Options

- **`/odom` distance check** (course suggestion): odometry is in the `odom` frame and drifts from the `map` frame the zones are defined in, so the "reached" check can silently never fire or fire early. It also duplicates move_base's goal tolerance with a second, independent threshold.
- **`/amcl_pose` distance check**: frame-correct, but still a second Arrival definition that can disagree with move_base (e.g. AMCL says "within 0.3 m" while move_base is still rotating to the goal yaw).
- **ROS parameter flags**: polling-based and not observable with `rostopic echo`.

## Consequences

- `add_markers` in `reactive` mode depends on `pick_objects` running; `add_marker.sh` therefore uses the separate `timed` mode.
- The topic is latched so start-up order between the two nodes does not matter.
