# Home Service Robot

A simulated household robot that collects a Virtual Object in one room and delivers it to another, autonomously.

## Places

**Pickup Zone**:
The place in the home where the Virtual Object waits to be collected.
_Avoid_: pickup goal, source, start zone

**Drop-off Zone**:
The place in the home where the Virtual Object is delivered.
_Avoid_: dropoff goal, destination, target zone

## The task

**Virtual Object**:
The item the robot carries from the Pickup Zone to the Drop-off Zone. It has no physical presence in the simulated world; it exists only as something drawn for the operator to see.
_Avoid_: package, item, marker (the marker is how it is drawn, not what it is)

**Delivery**:
One complete trip: travel to the Pickup Zone, collect the Virtual Object, travel to the Drop-off Zone, release it.
_Avoid_: mission, job, run

**Arrival**:
The moment the robot's navigation reports that it has reached a zone. Arrival is decided by navigation, never by the robot's distance from the zone measured separately.
_Avoid_: reached (informal), proximity

## Delivery Phases

A Delivery is always in exactly one phase, and the robot is the sole authority on which.

**Heading to Pickup**:
The robot is travelling to the Pickup Zone; the Virtual Object is waiting there.

**At Pickup**:
The robot has Arrived at the Pickup Zone and is collecting the Virtual Object. The Virtual Object is now with the robot and no longer shown at the Pickup Zone.
_Avoid_: loading, picking

**Heading to Drop-off**:
The robot is carrying the Virtual Object toward the Drop-off Zone.
_Avoid_: carrying, in transit

**Delivered**:
The robot has Arrived at the Drop-off Zone and released the Virtual Object there. Terminal.

**Failed**:
The robot could not Arrive at a zone after its allowed attempts. The Virtual Object stays where it was last shown. Terminal.
_Avoid_: aborted, error
