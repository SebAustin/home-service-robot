#include <gtest/gtest.h>

#include <vector>

#include "add_markers/marker_policy.h"
#include "pick_objects/delivery_phase.h"

using add_markers::Placement;
using add_markers::TimedStep;
using pick_objects::DeliveryPhase;

// ---------------------------------------------------------------- Reactive mode

TEST(ReactivePlacement, ObjectWaitsAtPickupWhileRobotTravelsThere) {
  EXPECT_EQ(Placement::AtPickup,
            add_markers::nextPlacement(Placement::AtPickup, DeliveryPhase::HeadingToPickup));
}

TEST(ReactivePlacement, ObjectDisappearsWhenRobotArrivesAtPickup) {
  EXPECT_EQ(Placement::Hidden,
            add_markers::nextPlacement(Placement::AtPickup, DeliveryPhase::AtPickup));
}

TEST(ReactivePlacement, ObjectStaysHiddenWhileBeingCarried) {
  EXPECT_EQ(Placement::Hidden,
            add_markers::nextPlacement(Placement::Hidden, DeliveryPhase::HeadingToDropoff));
}

TEST(ReactivePlacement, ObjectAppearsAtDropoffOnDelivery) {
  EXPECT_EQ(Placement::AtDropoff,
            add_markers::nextPlacement(Placement::Hidden, DeliveryPhase::Delivered));
}

TEST(ReactivePlacement, FailureLeavesObjectWhereItWasLastShown) {
  EXPECT_EQ(Placement::AtPickup,
            add_markers::nextPlacement(Placement::AtPickup, DeliveryPhase::Failed));
  EXPECT_EQ(Placement::Hidden,
            add_markers::nextPlacement(Placement::Hidden, DeliveryPhase::Failed));
}

TEST(ReactivePlacement, InitialPlacementIsAtPickup) {
  EXPECT_EQ(Placement::AtPickup, add_markers::initialPlacement());
}

// ---------------------------------------------------------------- Timed mode

TEST(TimedSchedule, ShowWaitHideWaitShowAtDropoff) {
  const std::vector<TimedStep> steps = add_markers::timedSchedule(5.0);
  ASSERT_EQ(3u, steps.size());

  EXPECT_EQ(Placement::AtPickup, steps[0].placement);
  EXPECT_DOUBLE_EQ(5.0, steps[0].holdSeconds);

  EXPECT_EQ(Placement::Hidden, steps[1].placement);
  EXPECT_DOUBLE_EQ(5.0, steps[1].holdSeconds);

  EXPECT_EQ(Placement::AtDropoff, steps[2].placement);
  EXPECT_LT(steps[2].holdSeconds, 0.0);  // negative = hold forever
}

TEST(TimedSchedule, PlacementAtElapsedTime) {
  const std::vector<TimedStep> steps = add_markers::timedSchedule(5.0);
  EXPECT_EQ(Placement::AtPickup, add_markers::placementAt(steps, 0.0));
  EXPECT_EQ(Placement::AtPickup, add_markers::placementAt(steps, 4.99));
  EXPECT_EQ(Placement::Hidden, add_markers::placementAt(steps, 5.0));
  EXPECT_EQ(Placement::Hidden, add_markers::placementAt(steps, 9.99));
  EXPECT_EQ(Placement::AtDropoff, add_markers::placementAt(steps, 10.0));
  EXPECT_EQ(Placement::AtDropoff, add_markers::placementAt(steps, 1000.0));
}

TEST(TimedSchedule, NegativeElapsedTimeIsTreatedAsStart) {
  const std::vector<TimedStep> steps = add_markers::timedSchedule(5.0);
  EXPECT_EQ(Placement::AtPickup, add_markers::placementAt(steps, -1.0));
}

// ---------------------------------------------------------------- Mode parsing

TEST(Mode, ParsesKnownModes) {
  add_markers::Mode mode = add_markers::Mode::Timed;
  ASSERT_TRUE(add_markers::parseMode("reactive", &mode));
  EXPECT_EQ(add_markers::Mode::Reactive, mode);
  ASSERT_TRUE(add_markers::parseMode("timed", &mode));
  EXPECT_EQ(add_markers::Mode::Timed, mode);
}

TEST(Mode, RejectsUnknownMode) {
  add_markers::Mode mode = add_markers::Mode::Reactive;
  EXPECT_FALSE(add_markers::parseMode("odom", &mode));
  EXPECT_EQ(add_markers::Mode::Reactive, mode);
}

// catkin_add_gtest links gtest but not gtest_main, so each test binary provides its own main.
int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
