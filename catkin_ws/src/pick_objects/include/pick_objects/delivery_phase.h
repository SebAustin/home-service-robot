#ifndef PICK_OBJECTS_DELIVERY_PHASE_H
#define PICK_OBJECTS_DELIVERY_PHASE_H

#include <string>

namespace pick_objects {

// The Delivery Phases (see GLOSSARY.md). The robot is the sole authority on the current phase
// and announces it on /delivery_status (see docs/adr/0001-delivery-status-topic.md).
enum class DeliveryPhase {
  HeadingToPickup,
  AtPickup,
  HeadingToDropoff,
  Delivered,
  Failed,
};

// Topic carrying the phase name as std_msgs/String, latched.
extern const char* const kDeliveryStatusTopic;

// Wire names: HEADING_TO_PICKUP, AT_PICKUP, HEADING_TO_DROPOFF, DELIVERED, FAILED.
std::string toString(DeliveryPhase phase);

// Parses a wire name. Returns false and leaves *phase untouched for unknown names.
bool fromString(const std::string& name, DeliveryPhase* phase);

}  // namespace pick_objects

#endif  // PICK_OBJECTS_DELIVERY_PHASE_H
