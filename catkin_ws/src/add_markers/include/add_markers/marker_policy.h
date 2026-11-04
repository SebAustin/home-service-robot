#ifndef ADD_MARKERS_MARKER_POLICY_H
#define ADD_MARKERS_MARKER_POLICY_H

#include <string>
#include <vector>

#include "pick_objects/delivery_phase.h"

namespace add_markers {

// Where the Virtual Object is drawn. Hidden = deleted from RViz (it is with the robot).
enum class Placement { AtPickup, Hidden, AtDropoff };

// timed:    fixed show / hide / show sequence, no robot needed (add_marker.sh)
// reactive: follows the robot's Delivery Phase on /delivery_status (home_service.sh)
enum class Mode { Timed, Reactive };

bool parseMode(const std::string& name, Mode* mode);

// ---- reactive mode

Placement initialPlacement();

// Placement after the robot announces `phase`. Failed keeps the object where it was last shown.
Placement nextPlacement(Placement current, pick_objects::DeliveryPhase phase);

// ---- timed mode

struct TimedStep {
  Placement placement;
  double holdSeconds;  // negative = hold forever
};

// AtPickup for `pauseSeconds`, Hidden for `pauseSeconds`, then AtDropoff forever.
std::vector<TimedStep> timedSchedule(double pauseSeconds);

// Placement `elapsedSeconds` after the schedule started.
Placement placementAt(const std::vector<TimedStep>& steps, double elapsedSeconds);

}  // namespace add_markers

#endif  // ADD_MARKERS_MARKER_POLICY_H
