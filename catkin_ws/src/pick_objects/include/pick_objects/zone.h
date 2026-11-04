#ifndef PICK_OBJECTS_ZONE_H
#define PICK_OBJECTS_ZONE_H

#include <string>

namespace pick_objects {

// A place in the home, expressed in the map frame: Pickup Zone or Drop-off Zone.
struct Zone {
  std::string name;
  double x;
  double y;
  double yaw;  // radians, heading the robot should face on Arrival
};

// Builds a Zone, rejecting an empty name or any non-finite coordinate.
// Throws std::invalid_argument naming the offending field.
Zone makeZone(const std::string& name, double x, double y, double yaw);

// Planar heading -> quaternion (rotation about +z only). Writes z and w components.
void yawToQuaternion(double yaw, double* qz, double* qw);

}  // namespace pick_objects

#endif  // PICK_OBJECTS_ZONE_H
