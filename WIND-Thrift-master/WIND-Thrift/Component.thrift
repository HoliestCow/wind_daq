namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "Common.thrift"
include "UUID.thrift"

/** Common definitions required for all system components. 
 * This must be the first field in all definition structures.
 */
struct ComponentDefinition
{
	1: UUID.UUID componentId;  //Used to match data and configuration, must be unique in the system
	2: string componentName; //Probably should be unique or it will be hard to distinguish sensors in the UI
	3: string vendorName;
	4: string serialNumber;
}

struct GridPositionAndOrientation
{
	// Transformation between this sensor's coordinate frame and the navigation
	// sensor's coordinate frame is expressed as a rotation + translation.
	//
	// The rotation (R) in rotationQuaternion expresses the rotation between this
	// sensor's coordinate frame and the navigation sensor's coordinate frame.
	//
	// The translation (T) in gridPosition is the position of the origin of this 
	// sensor's coordinate frame in the navigation sensor's coordinate frame.
	//
	// A point P_sensor in this sensor's coordinate frame can be converted to the 
	// point P_nav in navigation sensor's coordinate frame by the formula:
	// P_nav = R * P_sensor + T
	//
	// For gamma sensors, the origin of the coordinate frame is at the center 
	// of the sensor. X is parallel to the depth (shortest) dimension , Y is 
	// parallel to the width dimension, and Z is parallel to the length (longest)
	// dimension. In the case of spheres, rotationQuaternion need not be specified.
	//
	// Units in meters
	8: Common.Vector gridPosition;  // must specify z
	9: Common.Quaternion rotation; 
}
