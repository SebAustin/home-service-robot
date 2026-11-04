#include "pick_objects/zone_params.h"

#include <stdexcept>

namespace pick_objects {

namespace {

double requireParam(const ros::NodeHandle& nh, const std::string& key) {
  double value = 0.0;
  if (!nh.getParam(key, value)) {
    throw std::runtime_error("missing ROS parameter '" + key +
                             "' (load catkin_ws/src/map/zones.yaml with rosparam)");
  }
  return value;
}

}  // namespace

Zone loadZoneParam(const ros::NodeHandle& nh, const std::string& name) {
  const std::string base = "/zones/" + name + "/";
  return makeZone(name, requireParam(nh, base + "x"), requireParam(nh, base + "y"),
                  requireParam(nh, base + "yaw"));
}

}  // namespace pick_objects
