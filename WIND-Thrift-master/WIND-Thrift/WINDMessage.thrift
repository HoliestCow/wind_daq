namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "GammaSensor.thrift"
include "NeutronSensor.thrift"
include "AlgorithmPayload.thrift"
include "Navigation.thrift"
include "NavigationSensor.thrift"
include "ContextSensor.thrift"
include "EnvironmentalSensor.thrift"
include "Marker.thrift"
include "UUID.thrift"

enum MessageSourceType
{
	CVRS,
	PTU,
	User,
	Other
}

struct Message
{
	/** POSIX Time * 1000 - should match data packet's time stamp */
	1: i64 timeStamp;
	2: UUID.UUID messageId;
	/** Time that the message was sent, timestamped with the sent time */
	3: i64 timeStampSent
	/** Where did this message originate */
	4: MessageSourceType messageSource;
	5: string messageText;
	/** Unique ID of sender (e.g., PTU) */
	6: UUID.UUID originatorId;
	/** Optional data that can be attached to message */
	7: optional list<GammaSensor.GammaSpectrumData> gammaSpectrumData;
    8: optional list<GammaSensor.GammaListData> gammaListData;
    9: optional list<GammaSensor.GammaGrossCountData> gammaGrossCountData;
    10: optional list<GammaSensor.GammaDoseData> gammaDoseData;
    /** Neutron data associated with this timestamp */
    11: optional list<NeutronSensor.NeutronListData> neutronListData;
    12: optional list<NeutronSensor.NeutronSpectrumData> neutronSpectrumData;
    13: optional list<NeutronSensor.NeutronGrossCountData> neutronGrossCountData;
	14: optional list<EnvironmentalSensor.EnvironmentalSensorData> environmentalData;
	15: optional list<NavigationSensor.NavigationData> navigationData;
	16: optional list<ContextSensor.ContextVideoData> videoData;
    17: optional list<ContextSensor.ContextPointCloudData> pointCloudData;
    18: optional list<ContextSensor.ContextVoxelData> voxelData;
    19: optional list<ContextSensor.ContextMeshData> meshData;
	20: optional list<AlgorithmPayload.AlgorithmData> algorithmData;
	21: optional list<Marker.Marker> markers;
	22: optional list<Navigation.BoundingBox> boundingBoxes;
}