#include "pick_objects/delivery_state_machine.h"

#include <stdexcept>
#include <string>

namespace pick_objects {

DeliveryStateMachine::DeliveryStateMachine(int maxAttemptsPerLeg)
    : maxAttemptsPerLeg_(maxAttemptsPerLeg), attempt_(1), phase_(DeliveryPhase::HeadingToPickup) {
  if (maxAttemptsPerLeg < 1) {
    throw std::invalid_argument("maxAttemptsPerLeg must be at least 1");
  }
}

ZoneKind DeliveryStateMachine::target() const {
  switch (phase_) {
    case DeliveryPhase::HeadingToPickup:
      return ZoneKind::Pickup;
    case DeliveryPhase::HeadingToDropoff:
      return ZoneKind::Dropoff;
    default:
      return ZoneKind::None;
  }
}

bool DeliveryStateMachine::isFinished() const {
  return phase_ == DeliveryPhase::Delivered || phase_ == DeliveryPhase::Failed;
}

void DeliveryStateMachine::requireTravelling(const char* event) const {
  if (target() == ZoneKind::None) {
    throw std::logic_error(std::string(event) + " is not valid in phase " + toString(phase_));
  }
}

void DeliveryStateMachine::onArrival() {
  requireTravelling("arrival");
  phase_ = (phase_ == DeliveryPhase::HeadingToPickup) ? DeliveryPhase::AtPickup
                                                      : DeliveryPhase::Delivered;
}

void DeliveryStateMachine::onNavigationFailure() {
  requireTravelling("navigation failure");
  if (attempt_ < maxAttemptsPerLeg_) {
    ++attempt_;
    return;
  }
  phase_ = DeliveryPhase::Failed;
}

void DeliveryStateMachine::onPickupComplete() {
  if (phase_ != DeliveryPhase::AtPickup) {
    throw std::logic_error("pickup complete is not valid in phase " + toString(phase_));
  }
  phase_ = DeliveryPhase::HeadingToDropoff;
  attempt_ = 1;
}

}  // namespace pick_objects
