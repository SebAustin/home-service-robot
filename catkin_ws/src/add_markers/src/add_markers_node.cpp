// add_markers: draws the Virtual Object as a cube in RViz.
//
//   ~mode:=timed     show at pickup, 5 s, hide, 5 s, show at drop-off (add_marker.sh)
//   ~mode:=reactive  follow the robot's Delivery Phase on /delivery_status (home_service.sh)
//
// The current state is re-sent at 2 Hz so RViz shows it even if RViz starts after this node.
//
// Parameters:
//   /zones/{pickup,dropoff}/{x,y,yaw}  required, from catkin_ws/src/map/zones.yaml
//   ~mode            string, required: "timed" or "reactive"
//   ~pause_seconds   double, default 5.0 (timed mode)

#include <ros/ros.h>
#include <std_msgs/String.h>
#include <visualization_msgs/Marker.h>

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "add_markers/marker_policy.h"
#include "pick_objects/delivery_phase.h"
#include "pick_objects/zone.h"
#include "pick_objects/zone_params.h"

namespace {

using add_markers::Mode;
using add_markers::Placement;
using pick_objects::Zone;

constexpr int kExitBadConfig = 2;
constexpr double kRepublishHz = 2.0;
constexpr double kCubeSide = 0.25;  // metres

const char* placementName(Placement placement) {
  switch (placement) {
    case Placement::AtPickup:
      return "at pickup zone";
    case Placement::AtDropoff:
      return "at drop-off zone";
    case Placement::Hidden:
    default:
      return "hidden (with the robot)";
  }
}

visualization_msgs::Marker makeMarker(Placement placement, const Zone& pickup, const Zone& dropoff) {
  visualization_msgs::Marker marker;
  marker.header.frame_id = "map";
  marker.header.stamp = ros::Time::now();
  marker.ns = "virtual_object";
  marker.id = 0;
  marker.type = visualization_msgs::Marker::CUBE;

  if (placement == Placement::Hidden) {
    marker.action = visualization_msgs::Marker::DELETE;
    return marker;
  }

  const Zone& zone = (placement == Placement::AtPickup) ? pickup : dropoff;
  marker.action = visualization_msgs::Marker::ADD;
  marker.pose.position.x = zone.x;
  marker.pose.position.y = zone.y;
  marker.pose.position.z = kCubeSide / 2.0;  // resting on the floor
  marker.pose.orientation.w = 1.0;
  marker.scale.x = kCubeSide;
  marker.scale.y = kCubeSide;
  marker.scale.z = kCubeSide;
  // Amber, the same at both zones: it is one object that moves.
  marker.color.r = 1.0f;
  marker.color.g = 0.75f;
  marker.color.b = 0.0f;
  marker.color.a = 1.0f;
  marker.lifetime = ros::Duration(0);  // never expires
  return marker;
}

class VirtualObjectDisplay {
 public:
  VirtualObjectDisplay(ros::NodeHandle& nh, Mode mode, double pauseSeconds, Zone pickup,
                       Zone dropoff)
      : mode_(mode),
        schedule_(add_markers::timedSchedule(pauseSeconds)),
        pickup_(std::move(pickup)),
        dropoff_(std::move(dropoff)),
        placement_(add_markers::initialPlacement()),
        markerPublisher_(nh.advertise<visualization_msgs::Marker>("visualization_marker", 10)) {
    if (mode_ == Mode::Reactive) {
      statusSubscriber_ = nh.subscribe(pick_objects::kDeliveryStatusTopic, 10,
                                       &VirtualObjectDisplay::onStatus, this);
    }
    timer_ = nh.createTimer(ros::Duration(1.0 / kRepublishHz), &VirtualObjectDisplay::onTick, this);
    ROS_INFO("Virtual Object %s", placementName(placement_));
  }

 private:
  void onStatus(const std_msgs::String::ConstPtr& message) {
    pick_objects::DeliveryPhase phase;
    if (!pick_objects::fromString(message->data, &phase)) {
      ROS_WARN("Ignoring unknown delivery status '%s'", message->data.c_str());
      return;
    }
    setPlacement(add_markers::nextPlacement(placement_, phase));
  }

  void onTick(const ros::TimerEvent&) {
    if (mode_ == Mode::Timed) {
      // Simulated time is 0 until /clock arrives; start the schedule on the first real tick.
      if (start_.isZero()) {
        start_ = ros::Time::now();
      }
      setPlacement(add_markers::placementAt(schedule_, (ros::Time::now() - start_).toSec()));
    }
    markerPublisher_.publish(makeMarker(placement_, pickup_, dropoff_));
  }

  void setPlacement(Placement placement) {
    if (placement == placement_) {
      return;
    }
    placement_ = placement;
    ROS_INFO("Virtual Object %s", placementName(placement_));
    markerPublisher_.publish(makeMarker(placement_, pickup_, dropoff_));
  }

  const Mode mode_;
  const std::vector<add_markers::TimedStep> schedule_;
  const Zone pickup_;
  const Zone dropoff_;
  Placement placement_;
  ros::Time start_;
  ros::Publisher markerPublisher_;
  ros::Subscriber statusSubscriber_;
  ros::Timer timer_;
};

}  // namespace

int main(int argc, char** argv) {
  ros::init(argc, argv, "add_markers");
  ros::NodeHandle nh;
  ros::NodeHandle pnh("~");

  try {
    std::string modeName;
    Mode mode = Mode::Timed;
    if (!pnh.getParam("mode", modeName) || !add_markers::parseMode(modeName, &mode)) {
      throw std::runtime_error("~mode must be 'timed' or 'reactive', got '" + modeName + "'");
    }
    const double pauseSeconds = pnh.param("pause_seconds", 5.0);
    if (pauseSeconds < 0.0) {
      throw std::runtime_error("~pause_seconds must be >= 0");
    }
    VirtualObjectDisplay display(nh, mode, pauseSeconds, pick_objects::loadZoneParam(nh, "pickup"),
                                 pick_objects::loadZoneParam(nh, "dropoff"));
    ros::spin();
  } catch (const std::exception& error) {
    ROS_FATAL("add_markers: %s", error.what());
    return kExitBadConfig;
  }
  return 0;
}
