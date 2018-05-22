namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "Health.thrift"
include "Context.thrift"
include "Component.thrift"
include "UUID.thrift"
include "Common.thrift"



/**
 * Applies to camera and video
 * Optional distortion corrections:  x1=x(1+K1*r^2+K2*r^4+...)+2*P1*x*y+P2*(r^2+2x^2)], r^2=x^2+y^2
 * Unit conversion:                  xc=[fx*x1+cx*z]
 * The directions of x and y used in the definitions of CameraIntrinsics and ContextVideoConfiguration MUST 
 * comply with rotations specified within ComponentLocation.
 */
struct CameraIntrinsics
{
	1: UUID.UUID componentId;
	2: double cx; //horizontal optical center (pixel #)
	3: double cy; //vertical optical center (pixel #)
	4: double fx; //x focal length in pixels
	5: double fy; //y focal length in pixels
	6: optional list<double> RadialDistortionCoefficients; //one or more radial distortion coefficients, K in the above equation
	7: optional list<double> TangentialDistortionCoefficients; //one or more tangential distortion coefficients, P in the above equation
}


struct ContextVideoConfiguration
{
	1: UUID.UUID componentId;	
	2: string fileName;
	3: double framesPerSecond;  //-1 for triggered
	4: double verticalResolution; //pixels
	5: double horizontalResolution;
	6: Component.GridPositionAndOrientation componentPositionAndOrientation;
	7: double verticalFOV; // degrees
	8: double horizontalFOV;
	9: bool isRectified;
	10: bool isDeBayered;
	11: optional CameraIntrinsics intrinsics; //Required if isRectified is false
	12: i64 timeStamp;
}

struct ContextVideoDefinition
{
    1: Component.ComponentDefinition component;
    5: ContextVideoConfiguration videoConfiguration; // Starting configuration
}

struct ContextVideoData
{
	1: UUID.UUID componentId;
	2: i64 timeStamp; // POSIX Time * 1000
	3: Health.Health health; 
	/** Used to synchronize time between context sensor and PTU time in the case of an offset */ 
	4: i64 sensorTimeStamp;  //must be time of exposure, not time of file writing
	/** Only send if configuration has changed */
	5: optional ContextVideoConfiguration contextVideoConfiguration;
	6: binary cameraImage;
}

struct Context3DConfiguration
{
	1: UUID.UUID componentId;  // Used to match data, configuration, and defintion.  Must be unique in the system	
	2: Component.GridPositionAndOrientation componentPositionAndOrientation;
	3: i64 timeStamp;
}

struct Context3DDefinition
{
    1: Component.ComponentDefinition component;
    11: optional Context3DConfiguration configuration; // Starting configuration
}

struct ContextPointCloudData
{
	1: UUID.UUID componentId;
	2: i64 timeStamp; // POSIX Time * 1000
	3: Health.Health health;
	/** Used to synchronize time between context sensor and PTU time in the case of an offset */ 
	4: i64 sensorTimeStamp;
	/** For CSS->CMD synchronization */
	5: i64 syncFrame;
	/** Only send if configuration has changed */
	// 6: optional Context3DConfiguration pointCloudConfiguration;
	/** 3D Point Cloud */
	7: optional Context.PointCloud pointCloud;
}

struct ContextVoxelData
{
	1: UUID.UUID componentId;
	2: i64 timeStamp; // POSIX Time * 1000
	3: Health.Health health;
	/** Used to synchronize time between context sensor and PTU time in the case of an offset */ 
	4: i64 sensorTimeStamp;
	/** Only send if configuration has changed */
	// 5: optional Context3DConfiguration voxelConfiguration;
	/** 3D Voxel Map */
	6: optional Context.VoxelMap voxelMap;
}

struct ContextMeshData
{
	1: UUID.UUID componentId;
	2: i64 timeStamp; // POSIX Time * 1000
	3: Health.Health health; 
	/** Used to synchronize time between context sensor and PTU time in the case of an offset */
	4: i64 sensorTimeStamp;
	/** Only send if configuration has changed */
	// 5: optional Context3DConfiguration meshConfiguration;
	/** Batch produced with start and stop */
	6: optional Context.Mesh mesh;
}

union SensorModalityConfiguration
{
	1: ContextVideoConfiguration videoConfiguration;
	2: Context3DConfiguration threeDConfiguration;
	// Future modalities added here
}

/** ContextStreamConfiguration acts as a wrapper on the existing physical sensor configuration thrift objects
 *  and allows us to define a stream of video data, a stream of 3D data, etc. without duplicating objects in the 
 *  ICD. This structure allows us to future-proof the ICD; as new sensor modalities get added to the ICD 
 *  (e.g., a future audio sensor), a data stream of that modality could be expressed with this structure. By providing 
 *  a stream configuration wrapper that includes a physical sensor configuration union as opposed to just the union, 
 *  we allow for future stream-specific configuration fields to be added as well.
 */
struct ContextStreamConfiguration
{
	1: UUID.UUID componentId;
	2: Component.GridPositionAndOrientation componentPositionAndOrientation;
	3: SensorModalityConfiguration modalityConfiguration;
	4: i64 timeStamp;
}



/** ContextStreamDefinition provides a generic way of specifying any 
 *  streaming data source. The definition MUST include the format of the stream, 
 *  a version of that format, the address where it can be accessed, and a URI 
 *  where documentation on how to process the format can be found. One physical 
 *  sensor may produce multiple stream components.
 */
struct ContextStreamDefinition
{
	1: Component.ComponentDefinition component;
	2: string streamFormat;
	3: string streamAddress;
	4: string formatVersion;
	5: string documentationURI;
	66: optional ContextStreamConfiguration configuration; // Optional starting configuration
}

/** If a stream is live, ContextStreamIndexData for it MUST be provided with each DataPayload and 
 *  provides a mechanism for syncing the current DataPayload timestamp with a 
 *  timestamp in the stream.
 */
struct ContextStreamIndexData {
	1: UUID.UUID componentId;
	2: i64 timeStamp;
	3: i64 streamTimeStamp;
}