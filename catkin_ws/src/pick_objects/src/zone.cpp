#include "pick_objects/zone.h"

#include <cmath>
#include <stdexcept>

namespace pick_objects {

namespace {

void requireFinite(const std::string& zone, const char* field, double value) {
  if (!std::isfinite(value)) {
    throw std::invalid_argument("zone '" + zone + "': " + field + " must be a finite number");
  }
}

}  // namespace

Zone makeZone(const std::string& name, double x, double y, double yaw) {
  if (name.empty()) {
    throw std::invalid_argument("zone name must not be empty");
  }
  requireFinite(name, "x", x);
  requireFinite(name, "y", y);
  requireFinite(name, "yaw", yaw);
  return Zone{name, x, y, yaw};
}

void yawToQuaternion(double yaw, double* qz, double* qw) {
  *qz = std::sin(yaw / 2.0);
  *qw = std::cos(yaw / 2.0);
}

}  // namespace pick_objects
