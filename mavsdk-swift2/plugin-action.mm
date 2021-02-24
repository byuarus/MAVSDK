#import "plugin-action.h"
#import "mavsdk-swift2-impl.h"

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>


MAVSDKActionOrbitYawBehavior translateFromCppOrbitYawBehavior(mavsdk::Action::OrbitYawBehavior orbit_yaw_behavior)
{
    switch (orbit_yaw_behavior) {
        default:
            NSLog(@"Unknown orbit_yaw_behavior enum value: %d", static_cast<int>(orbit_yaw_behavior));
        // FALLTHROUGH
        case mavsdk::Action::OrbitYawBehavior::HoldFrontToCircleCenter:
            return MAVSDKActionOrbitYawBehaviorHoldFrontToCircleCenter;
        case mavsdk::Action::OrbitYawBehavior::HoldInitialHeading:
            return MAVSDKActionOrbitYawBehaviorHoldInitialHeading;
        case mavsdk::Action::OrbitYawBehavior::Uncontrolled:
            return MAVSDKActionOrbitYawBehaviorUncontrolled;
        case mavsdk::Action::OrbitYawBehavior::HoldFrontTangentToCircle:
            return MAVSDKActionOrbitYawBehaviorHoldFrontTangentToCircle;
        case mavsdk::Action::OrbitYawBehavior::RcControlled:
            return MAVSDKActionOrbitYawBehaviorRcControlled;
    }
}

mavsdk::Action::OrbitYawBehavior translateToCppOrbitYawBehavior(MAVSDKActionOrbitYawBehavior orbitYawBehavior)
{
    switch (orbitYawBehavior) {
        default:
            NSLog(@"Unknown OrbitYawBehavior enum value: %d", static_cast<int>(orbitYawBehavior));
        // FALLTHROUGH
        case MAVSDKActionOrbitYawBehaviorHoldFrontToCircleCenter:
            return mavsdk::Action::OrbitYawBehavior::HoldFrontToCircleCenter;
        case MAVSDKActionOrbitYawBehaviorHoldInitialHeading:
            return mavsdk::Action::OrbitYawBehavior::HoldInitialHeading;
        case MAVSDKActionOrbitYawBehaviorUncontrolled:
            return mavsdk::Action::OrbitYawBehavior::Uncontrolled;
        case MAVSDKActionOrbitYawBehaviorHoldFrontTangentToCircle:
            return mavsdk::Action::OrbitYawBehavior::HoldFrontTangentToCircle;
        case MAVSDKActionOrbitYawBehaviorRcControlled:
            return mavsdk::Action::OrbitYawBehavior::RcControlled;
    }
}








@implementation MAVSDKAction

mavsdk::Action *action;


+(id)alloc{
    return [super alloc];
}

- (id)initWithMavsdkSwift2Impl:(MavsdkSwift2Impl*)mavsdkSwift2Impl {
    action = new mavsdk::Action(*[mavsdkSwift2Impl mavsdkSystem]);
    return [super init];
}

- (MAVSDKActionResult)arm {
    return (MAVSDKActionResult)action->arm();
}
- (MAVSDKActionResult)disarm {
    return (MAVSDKActionResult)action->disarm();
}
- (MAVSDKActionResult)takeoff {
    return (MAVSDKActionResult)action->takeoff();
}
- (MAVSDKActionResult)land {
    return (MAVSDKActionResult)action->land();
}
- (MAVSDKActionResult)reboot {
    return (MAVSDKActionResult)action->reboot();
}
- (MAVSDKActionResult)shutdown {
    return (MAVSDKActionResult)action->shutdown();
}
- (MAVSDKActionResult)terminate {
    return (MAVSDKActionResult)action->terminate();
}
- (MAVSDKActionResult)kill {
    return (MAVSDKActionResult)action->kill();
}
- (MAVSDKActionResult)returnToLaunch {
    return (MAVSDKActionResult)action->return_to_launch();
}
- (MAVSDKActionResult)gotoLocation:(double)latitudeDeg :(double)longitudeDeg :(float)absoluteAltitudeM :(float)yawDeg {
    return (MAVSDKActionResult)action->goto_location(latitudeDeg, longitudeDeg, absoluteAltitudeM, yawDeg);
}
- (MAVSDKActionResult)doOrbit:(float)radiusM :(float)velocityMs :(MAVSDKActionOrbitYawBehavior)yawBehavior :(double)latitudeDeg :(double)longitudeDeg :(double)absoluteAltitudeM {
    return (MAVSDKActionResult)action->do_orbit(radiusM, velocityMs, (mavsdk::Action::OrbitYawBehavior)yawBehavior, latitudeDeg, longitudeDeg, absoluteAltitudeM);
}
- (MAVSDKActionResult)transitionToFixedwing {
    return (MAVSDKActionResult)action->transition_to_fixedwing();
}
- (MAVSDKActionResult)transitionToMulticopter {
    return (MAVSDKActionResult)action->transition_to_multicopter();
}

- (MAVSDKActionResult)setTakeoffAltitude:(float)altitude {
    return (MAVSDKActionResult)action->set_takeoff_altitude(altitude);
}

- (MAVSDKActionResult)setMaximumSpeed:(float)speed {
    return (MAVSDKActionResult)action->set_maximum_speed(speed);
}

- (MAVSDKActionResult)setReturnToLaunchAltitude:(float)relativeAltitudeM {
    return (MAVSDKActionResult)action->set_return_to_launch_altitude(relativeAltitudeM);
}


@end
