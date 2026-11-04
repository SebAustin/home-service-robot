#ifndef PICK_OBJECTS_ZONE_PARAMS_H
#define PICK_OBJECTS_ZONE_PARAMS_H

#include <ros/node_handle.h>

#include <string>

#include "pick_objects/zone.h"

namespace pick_objects {

// Reads /zones/<name>/{x,y,yaw} (loaded from catkin_ws/src/map/zones.yaml by the shell scripts).
// Both pick_objects and add_markers read the zones through this one function, so their
// coordinates cannot drift apart. Throws std::runtime_error naming the missing parameter.
Zone loadZoneParam(const ros::NodeHandle& nh, const std::string& name);

}  // namespace pick_objects

#endif  // PICK_OBJECTS_ZONE_PARAMS_H
