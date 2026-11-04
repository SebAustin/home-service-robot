#include "add_markers/marker_policy.h"

namespace add_markers {

using pick_objects::DeliveryPhase;

bool parseMode(const std::string& name, Mode* mode) {
  if (name == "timed") {
    *mode = Mode::Timed;
    return true;
  }
  if (name == "reactive") {
    *mode = Mode::Reactive;
    return true;
  }
  return false;
}

Placement initialPlacement() { return Placement::AtPickup; }

Placement nextPlacement(Placement current, DeliveryPhase phase) {
  switch (phase) {
    case DeliveryPhase::HeadingToPickup:
      return Placement::AtPickup;
    case DeliveryPhase::AtPickup:
    case DeliveryPhase::HeadingToDropoff:
      return Placement::Hidden;
    case DeliveryPhase::Delivered:
      return Placement::AtDropoff;
    case DeliveryPhase::Failed:
    default:
      return current;
  }
}

std::vector<TimedStep> timedSchedule(double pauseSeconds) {
  return {
      {Placement::AtPickup, pauseSeconds},
      {Placement::Hidden, pauseSeconds},
      {Placement::AtDropoff, -1.0},
  };
}

Placement placementAt(const std::vector<TimedStep>& steps, double elapsedSeconds) {
  double stepEnd = 0.0;
  for (const TimedStep& step : steps) {
    if (step.holdSeconds < 0.0) {
      return step.placement;
    }
    stepEnd += step.holdSeconds;
    if (elapsedSeconds < stepEnd) {
      return step.placement;
    }
  }
  return steps.empty() ? Placement::Hidden : steps.back().placement;
}

}  // namespace add_markers
