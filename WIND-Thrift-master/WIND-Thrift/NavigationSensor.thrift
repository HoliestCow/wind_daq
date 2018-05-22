namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "Health.thrift"
include "UUID.thrift"
include "Common.thrift"
include "Component.thrift"
include "Navigation.thrift"

enum FrameOfReference
{
	// Not currently used
	LocalNED,
	VehicleCarriedNED,
	
	// Body frame is fixed to the system, defined using the aerospace 
	// convention with a right-handed coordinate system where
	// X points forward, Y to the right, and Z down.
	//
	// These values are provided with respect to the current body frame
	//     velocityXYZ,
	//     accelerationXYZ,
	//     gravityCompAccelXYZ,
	//     angularRateXYZ, 
	//     magneticFieldXYZ, 
	//     and velocityErrorXYZ,    
	//
	// Position, position error, orientation, and orientation error
	// values are with respect to the current body frame and are 
	// typically not used in the navigation context.
	//
	// roll is a rotation about x; a positive roll is to the right
	// pitch is a rotation about y; a positive pitch points upwards
	// heading is a rotation about z; typically defined as yaw
	//
	Body,

	// Geodetic_NED (North, East, Down)
	// Latitude, longitude, altitude are in the geodetic 
	// datum specified by NavigationSensorDefinition.datum
	// 
	// X, Y, Z map to North, East, Down in the vehicle-carried NED frame for 
	//     velocityXYZ,
	//     accelerationXYZ,
	//     gravityCompAccelXYZ, 
	//     magneticFieldXYZ, 
	//     positionErrorXYZ, 
	//     and velocityErrorXYZ,
	// 
	// Orientation, angular rate, and orientation error are in 
	// the vehicle-carried NED frame, with X pointing forward, 
	// Y to the right, and Z down.
	//
	// roll is a rotation about x; a positive roll is to the right
	// pitch is a rotation about y; a positive pitch points upwards
	// heading is a rotation about z; 0 is north, 90 is east
	// 
	Geodetic_NED,

	// Geodetic_ECEF (earth-centered, earth-fixed)
	// Latitude, longitude, altitude are in the geodetic 
	// datum specified by NavigationSensorDefinition.datum
	//
	// X, Y, Z are in the ECEF frame for
	//     position (x, y, z)
	//     velocityXYZ,
	//     accelerationXYZ,
	//     gravityCompAccelXYZ, 
	//     magneticFieldXYZ, 
	//     positionErrorXYZ, 
	//     and velocityErrorXYZ
	// 
	// Orientation, angular rate, and orientation error are also referenced to 
	// the ECEF frame, with the origin at the center of the earth, X pointing 
	// to the intersection of the equator and the Prime Meridian, Y pointing 
	// to the intersection of the equator and the 90 degree line of longitude, 
	// and Z pointing in the direction of the north pole.
	//
	// roll is a rotation about x
	// pitch is a rotation about y
	// heading is a rotation about z
	//
	Geodetic_ECEF,

	// LocalXYZ is a custom reference frame. This is used to 
	// reference a body frame at a specific time, specified in the 
	// referenceFrameTimestamp field
	//
	LocalXYZ
}

enum GeodeticDatum
{
	AGD84,
	GRS80,
	NAD83,
	OSGB36,
	PZ9011,
	WGS84
}

enum GPSFixType
{
	NotAvailable,
	TimeOnly,
	TwoD,
	ThreeD
}

/*
 * Defines the navigation output provided by any navigation sensor or algorithm.
 */
struct NavigationOutputDefinition
{
	1: FrameOfReference sensorFrameOfReference = FrameOfReference.Geodetic_NED;  // Default is set up for GPS, not compass
	2: GeodeticDatum datum = GeodeticDatum.WGS84;  // Datum is (mostly) irrelevant for non GPS/INS applications
	3: bool hasLatitude = false;
	4: bool hasLongitude = false;
	5: bool hasAltitude = false;
	6: bool hasX = false;
	7: bool hasY = false;
	8: bool hasZ = false;
	9: bool hasAccelerationX = false;
	10: bool hasAccelerationY = false;
	11: bool hasAccelerationZ = false;
	12: bool hasNumberOfSatellites = false;
	13: bool hasQualityOfFix = false;
	14: bool hasPitch = false;
	15: bool hasRoll = false;
	16: bool hasHeading = false;
	17: bool hasVelocityX = false;
	18: bool hasVelocityY = false;
	19: bool hasVelocityZ = false;
	20: bool hasSpeed = false;
}


/*
 * Defines the physical navigation sensor and its output.  This should not change during normal operation.
 */
struct NavigationSensorDefinition
{
	1: Component.ComponentDefinition component;
	2: NavigationOutputDefinition navOutputDefinition;
}

/** What do we need to send the Vendors?**/
struct NavigationData
{
	1: UUID.UUID componentId;
	2: i64 timeStamp;  // POSIX time in milliseconds
	3: Health.Health health;

	// Position
	4: optional Navigation.Location location

	// Non-geodetic position data
	7: optional Common.Vector position; // meters

	// Acceleration
	10: optional Common.Vector acceleration; // m/s%2

	// GPS 
	13: optional i32 numberOfSatellites;
	14: optional GPSFixType qualityOfFix;

	// Orientation
	15: optional Common.NauticalAngles orientation;
	
	// Velocity
	18: optional Common.Vector velocity; // meters per second
	21: optional double speed;     // meters per second

	// Angular rate
	22: optional Common.Vector angularRate; // degrees per second

	// Magnetic field
	25: optional Common.Vector magneticField; // Gauss

	// Gravity compensated acceleration
	28: optional Common.Vector gravityCompAccel; // m/s^2

	// Estimated position error
	31: optional Common.Vector positionErrorXYZ; // meters
	34: optional double positionError;  // meters

	// Estimated velocity error
	35: optional Common.Vector velocityError; // meters per second
	38: optional double speedError;  // meters per second

	// Estimated orientation error
	39: optional Common.NauticalAngles orientationError;

	// Estimated time error
	42: optional i64 timeError; // nanoseconds

	// Timestamp of reference frame
	// Only used by LocalXYZ
	43: optional i64 referenceFrameTimeStamp;  // POSIX time in milliseconds
}
