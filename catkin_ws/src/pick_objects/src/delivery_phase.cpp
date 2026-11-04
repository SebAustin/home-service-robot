#include "pick_objects/delivery_phase.h"

#include <utility>

namespace pick_objects {

const char* const kDeliveryStatusTopic = "/delivery_status";

namespace {

const std::pair<DeliveryPhase, const char*> kNames[] = {
    {DeliveryPhase::HeadingToPickup, "HEADING_TO_PICKUP"},
    {DeliveryPhase::AtPickup, "AT_PICKUP"},
    {DeliveryPhase::HeadingToDropoff, "HEADING_TO_DROPOFF"},
    {DeliveryPhase::Delivered, "DELIVERED"},
    {DeliveryPhase::Failed, "FAILED"},
};

}  // namespace

std::string toString(DeliveryPhase phase) {
  for (const auto& entry : kNames) {
    if (entry.first == phase) {
      return entry.second;
    }
  }
  return "UNKNOWN";  // unreachable while kNames covers every enumerator
}

bool fromString(const std::string& name, DeliveryPhase* phase) {
  for (const auto& entry : kNames) {
    if (name == entry.second) {
      *phase = entry.first;
      return true;
    }
  }
  return false;
}

}  // namespace pick_objects
