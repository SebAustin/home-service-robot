// pick_objects: drives one Delivery by sending the Pickup Zone and Drop-off Zone to move_base,
// and announces every Delivery Phase on /delivery_status (latched std_msgs/String).
//
// Parameters:
//   /zones/{pickup,dropoff}/{x,y,yaw}  required, from catkin_ws/src/map/zones.yaml
//   ~max_attempts        int,    default 3    navigation attempts per leg before FAILED
//   ~pickup_seconds      double, default 5.0  time spent At Pickup
//   ~goal_timeout        double, default 180  seconds before an unfinished goal counts as failed
//
// Exit code: 0 Delivered, 1 Failed, 2 bad configuration.

#include <actionlib/client/simple_action_client.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <ros/ros.h>
#include <std_msgs/String.h>

#include <exception>
#include <stdexcept>
#include <string>

#include "pick_objects/delivery_phase.h"
#include "pick_objects/delivery_state_machine.h"
#include "pick_objects/zone.h"
#include "pick_objects/zone_params.h"

namespace {

using MoveBaseClient = actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction>;
using pick_objects::DeliveryPhase;
using pick_objects::DeliveryStateMachine;
using pick_objects::Zone;
using pick_objects::ZoneKind;

constexpr int kExitDelivered = 0;
constexpr int kExitFailed = 1;
constexpr int kExitBadConfig = 2;
constexpr double kServerWaitLogSeconds = 5.0;

struct Settings {
  Zone pickup;
  Zone dropoff;
  int maxAttempts;
  double pickupSeconds;
  double goalTimeout;
};

Settings loadSettings(const ros::NodeHandle& nh, const ros::NodeHandle& pnh) {
  Settings settings{pick_objects::loadZoneParam(nh, "pickup"),
                    pick_objects::loadZoneParam(nh, "dropoff"), pnh.param("max_attempts", 3),
                    pnh.param("pickup_seconds", 5.0), pnh.param("goal_timeout", 180.0)};
  if (settings.maxAttempts < 1 || settings.pickupSeconds < 0.0 || settings.goalTimeout <= 0.0) {
    throw std::runtime_error("need ~max_attempts >= 1, ~pickup_seconds >= 0, ~goal_timeout > 0");
  }
  return settings;
}

move_base_msgs::MoveBaseGoal makeGoal(const Zone& zone) {
  move_base_msgs::MoveBaseGoal goal;
  goal.target_pose.header.frame_id = "map";
  goal.target_pose.header.stamp = ros::Time::now();
  goal.target_pose.pose.position.x = zone.x;
  goal.target_pose.pose.position.y = zone.y;
  pick_objects::yawToQuaternion(zone.yaw, &goal.target_pose.pose.orientation.z,
                                &goal.target_pose.pose.orientation.w);
  return goal;
}

// Sends the goal and blocks until move_base finishes or the timeout expires.
bool navigateTo(MoveBaseClient& client, const Zone& zone, int attempt, double timeoutSeconds) {
  ROS_INFO("Heading to %s zone (%.2f, %.2f), attempt %d", zone.name.c_str(), zone.x, zone.y,
           attempt);
  client.sendGoal(makeGoal(zone));
  if (!client.waitForResult(ros::Duration(timeoutSeconds))) {
    client.cancelGoal();
    ROS_WARN("Gave up on %s zone after %.0f s", zone.name.c_str(), timeoutSeconds);
    return false;
  }
  const actionlib::SimpleClientGoalState state = client.getState();
  if (state != actionlib::SimpleClientGoalState::SUCCEEDED) {
    ROS_WARN("move_base did not reach %s zone: %s", zone.name.c_str(), state.toString().c_str());
    return false;
  }
  return true;
}

class StatusAnnouncer {
 public:
  explicit StatusAnnouncer(ros::NodeHandle& nh)
      : publisher_(nh.advertise<std_msgs::String>(pick_objects::kDeliveryStatusTopic, 1,
                                                  /*latch=*/true)) {}

  void announce(DeliveryPhase phase) {
    std_msgs::String message;
    message.data = pick_objects::toString(phase);
    publisher_.publish(message);
    ROS_INFO("Delivery phase: %s", message.data.c_str());
  }

 private:
  ros::Publisher publisher_;
};

int runDelivery(const Settings& settings, MoveBaseClient& client, StatusAnnouncer& status) {
  DeliveryStateMachine delivery(settings.maxAttempts);
  std::string lastZoneName;
  status.announce(delivery.phase());

  while (ros::ok() && !delivery.isFinished()) {
    if (delivery.phase() == DeliveryPhase::AtPickup) {
      ROS_INFO("Reached pickup zone");
      ros::Duration(settings.pickupSeconds).sleep();
      delivery.onPickupComplete();
      status.announce(delivery.phase());
      continue;
    }

    const Zone& zone = (delivery.target() == ZoneKind::Pickup) ? settings.pickup : settings.dropoff;
    lastZoneName = zone.name;
    const DeliveryPhase before = delivery.phase();
    if (navigateTo(client, zone, delivery.attempt(), settings.goalTimeout)) {
      delivery.onArrival();
    } else {
      delivery.onNavigationFailure();
    }
    if (delivery.phase() != before) {
      status.announce(delivery.phase());
    }
  }

  if (delivery.phase() == DeliveryPhase::Delivered) {
    ROS_INFO("Reached drop-off zone");
    return kExitDelivered;
  }
  if (delivery.phase() == DeliveryPhase::Failed) {
    ROS_ERROR("Delivery failed: could not reach the %s zone after %d attempts",
              lastZoneName.c_str(), settings.maxAttempts);
  }
  return kExitFailed;
}

}  // namespace

int main(int argc, char** argv) {
  ros::init(argc, argv, "pick_objects");
  ros::NodeHandle nh;
  ros::NodeHandle pnh("~");

  Settings settings{};
  try {
    settings = loadSettings(nh, pnh);
  } catch (const std::exception& error) {
    ROS_FATAL("pick_objects: %s", error.what());
    return kExitBadConfig;
  }

  StatusAnnouncer status(nh);
  MoveBaseClient client("move_base", true);
  while (ros::ok() && !client.waitForServer(ros::Duration(kServerWaitLogSeconds))) {
    ROS_INFO("Waiting for the move_base action server to come up");
  }
  if (!ros::ok()) {
    return kExitFailed;
  }

  return runDelivery(settings, client, status);
}
