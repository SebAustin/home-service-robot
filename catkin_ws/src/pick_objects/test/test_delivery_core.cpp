#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

#include "pick_objects/delivery_phase.h"
#include "pick_objects/delivery_state_machine.h"
#include "pick_objects/zone.h"

using pick_objects::DeliveryPhase;
using pick_objects::DeliveryStateMachine;
using pick_objects::Zone;
using pick_objects::ZoneKind;

namespace {
const double kPi = std::acos(-1.0);  // M_PI is not defined under strict -std=c++14
}  // namespace

// ---------------------------------------------------------------- Zone

TEST(Zone, KeepsGivenPose) {
  const Zone zone = pick_objects::makeZone("pickup", -2.5, 1.9, 1.5708);
  EXPECT_EQ("pickup", zone.name);
  EXPECT_DOUBLE_EQ(-2.5, zone.x);
  EXPECT_DOUBLE_EQ(1.9, zone.y);
  EXPECT_DOUBLE_EQ(1.5708, zone.yaw);
}

TEST(Zone, RejectsNonFiniteCoordinates) {
  const double nan = std::numeric_limits<double>::quiet_NaN();
  const double inf = std::numeric_limits<double>::infinity();
  EXPECT_THROW(pick_objects::makeZone("pickup", nan, 0.0, 0.0), std::invalid_argument);
  EXPECT_THROW(pick_objects::makeZone("pickup", 0.0, inf, 0.0), std::invalid_argument);
  EXPECT_THROW(pick_objects::makeZone("pickup", 0.0, 0.0, nan), std::invalid_argument);
}

TEST(Zone, RejectsEmptyName) {
  EXPECT_THROW(pick_objects::makeZone("", 0.0, 0.0, 0.0), std::invalid_argument);
}

TEST(Zone, YawBecomesUnitQuaternionAboutZ) {
  const Zone zone = pick_objects::makeZone("dropoff", 0.0, 0.0, kPi / 2.0);
  double qz = 0.0;
  double qw = 0.0;
  pick_objects::yawToQuaternion(zone.yaw, &qz, &qw);
  EXPECT_NEAR(std::sin(kPi / 4.0), qz, 1e-9);
  EXPECT_NEAR(std::cos(kPi / 4.0), qw, 1e-9);
  EXPECT_NEAR(1.0, qz * qz + qw * qw, 1e-9);
}

// ---------------------------------------------------------------- Phase names (the wire contract)

TEST(DeliveryPhase, NamesMatchTheStatusContract) {
  EXPECT_EQ("HEADING_TO_PICKUP", pick_objects::toString(DeliveryPhase::HeadingToPickup));
  EXPECT_EQ("AT_PICKUP", pick_objects::toString(DeliveryPhase::AtPickup));
  EXPECT_EQ("HEADING_TO_DROPOFF", pick_objects::toString(DeliveryPhase::HeadingToDropoff));
  EXPECT_EQ("DELIVERED", pick_objects::toString(DeliveryPhase::Delivered));
  EXPECT_EQ("FAILED", pick_objects::toString(DeliveryPhase::Failed));
}

TEST(DeliveryPhase, ParsesEveryNameItPrints) {
  const DeliveryPhase all[] = {DeliveryPhase::HeadingToPickup, DeliveryPhase::AtPickup,
                               DeliveryPhase::HeadingToDropoff, DeliveryPhase::Delivered,
                               DeliveryPhase::Failed};
  for (const DeliveryPhase phase : all) {
    DeliveryPhase parsed = DeliveryPhase::Failed;
    ASSERT_TRUE(pick_objects::fromString(pick_objects::toString(phase), &parsed));
    EXPECT_EQ(phase, parsed);
  }
}

TEST(DeliveryPhase, RejectsUnknownName) {
  DeliveryPhase parsed = DeliveryPhase::Delivered;
  EXPECT_FALSE(pick_objects::fromString("at_pickup", &parsed));
  EXPECT_FALSE(pick_objects::fromString("", &parsed));
  EXPECT_EQ(DeliveryPhase::Delivered, parsed);  // untouched on failure
}

// ---------------------------------------------------------------- State machine

TEST(DeliveryStateMachine, StartsHeadingToPickup) {
  const DeliveryStateMachine delivery(3);
  EXPECT_EQ(DeliveryPhase::HeadingToPickup, delivery.phase());
  EXPECT_EQ(ZoneKind::Pickup, delivery.target());
  EXPECT_EQ(1, delivery.attempt());
  EXPECT_FALSE(delivery.isFinished());
}

TEST(DeliveryStateMachine, HappyPathVisitsEveryPhaseInOrder) {
  DeliveryStateMachine delivery(3);

  delivery.onArrival();
  EXPECT_EQ(DeliveryPhase::AtPickup, delivery.phase());
  EXPECT_EQ(ZoneKind::None, delivery.target());

  delivery.onPickupComplete();
  EXPECT_EQ(DeliveryPhase::HeadingToDropoff, delivery.phase());
  EXPECT_EQ(ZoneKind::Dropoff, delivery.target());
  EXPECT_EQ(1, delivery.attempt());

  delivery.onArrival();
  EXPECT_EQ(DeliveryPhase::Delivered, delivery.phase());
  EXPECT_EQ(ZoneKind::None, delivery.target());
  EXPECT_TRUE(delivery.isFinished());
}

TEST(DeliveryStateMachine, RetriesSameZoneUntilAttemptsRunOut) {
  DeliveryStateMachine delivery(3);

  delivery.onNavigationFailure();
  EXPECT_EQ(DeliveryPhase::HeadingToPickup, delivery.phase());
  EXPECT_EQ(2, delivery.attempt());

  delivery.onNavigationFailure();
  EXPECT_EQ(DeliveryPhase::HeadingToPickup, delivery.phase());
  EXPECT_EQ(3, delivery.attempt());

  delivery.onNavigationFailure();
  EXPECT_EQ(DeliveryPhase::Failed, delivery.phase());
  EXPECT_TRUE(delivery.isFinished());
  EXPECT_EQ(ZoneKind::None, delivery.target());
}

TEST(DeliveryStateMachine, AttemptsResetForTheDropoffLeg) {
  DeliveryStateMachine delivery(3);
  delivery.onNavigationFailure();  // pickup attempt 2
  delivery.onArrival();
  delivery.onPickupComplete();
  EXPECT_EQ(1, delivery.attempt());

  delivery.onNavigationFailure();
  delivery.onNavigationFailure();
  delivery.onNavigationFailure();
  EXPECT_EQ(DeliveryPhase::Failed, delivery.phase());
}

TEST(DeliveryStateMachine, RejectsEventsThatDoNotFitThePhase) {
  DeliveryStateMachine delivery(3);
  EXPECT_THROW(delivery.onPickupComplete(), std::logic_error);  // not at pickup yet

  delivery.onArrival();
  EXPECT_THROW(delivery.onArrival(), std::logic_error);  // already arrived
  EXPECT_THROW(delivery.onNavigationFailure(), std::logic_error);

  delivery.onPickupComplete();
  delivery.onArrival();
  EXPECT_THROW(delivery.onArrival(), std::logic_error);  // finished
  EXPECT_THROW(delivery.onNavigationFailure(), std::logic_error);
}

TEST(DeliveryStateMachine, RequiresAtLeastOneAttempt) {
  EXPECT_THROW(DeliveryStateMachine(0), std::invalid_argument);
}
