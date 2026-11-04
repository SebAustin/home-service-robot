#ifndef PICK_OBJECTS_DELIVERY_STATE_MACHINE_H
#define PICK_OBJECTS_DELIVERY_STATE_MACHINE_H

#include "pick_objects/delivery_phase.h"

namespace pick_objects {

// Which zone the robot should be navigating to right now.
enum class ZoneKind { None, Pickup, Dropoff };

// One Delivery, with no ROS in it: the node feeds it navigation outcomes and it decides the next
// phase. Each leg (to pickup, to drop-off) gets `maxAttemptsPerLeg` navigation attempts.
//
//   HeadingToPickup --arrival--> AtPickup --pickupComplete--> HeadingToDropoff --arrival--> Delivered
//         |  failure (attempts left): retry same leg                 |
//         +-- failure (no attempts left) --> Failed <----------------+
//
// Events that do not fit the current phase throw std::logic_error: they mean a bug in the caller.
class DeliveryStateMachine {
 public:
  explicit DeliveryStateMachine(int maxAttemptsPerLeg);

  DeliveryPhase phase() const { return phase_; }
  ZoneKind target() const;
  int attempt() const { return attempt_; }
  bool isFinished() const;

  void onArrival();
  void onNavigationFailure();
  void onPickupComplete();

 private:
  void requireTravelling(const char* event) const;

  int maxAttemptsPerLeg_;
  int attempt_;
  DeliveryPhase phase_;
};

}  // namespace pick_objects

#endif  // PICK_OBJECTS_DELIVERY_STATE_MACHINE_H
