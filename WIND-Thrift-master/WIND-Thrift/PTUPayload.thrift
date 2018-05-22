namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "Common.thrift"
include "GammaSensor.thrift"
include "NeutronSensor.thrift"
include "AlgorithmPayload.thrift"
include "NavigationSensor.thrift"
include "ContextSensor.thrift"
include "EnvironmentalSensor.thrift"
include "WINDMessage.thrift"
include "Navigation.thrift"
include "Health.thrift"
include "Marker.thrift"
include "UUID.thrift"
include "PassiveMaterial.thrift"

const string PROTOCOL_VERSION = "0.3.1";

enum RecordingType
{
    Other,
    Calibration,
    Measurement,
    Background,
    /** This encompasses mobile and portal operations */
    Search 
}

enum UnitType
{
    Wearable,
    Luggable,
    Portal,
    Stationary,
    Aerial,
    Vehicle,
    /** Unit is a source - for tracking source locations */
    Source,
    Other
}

struct RecordingConfiguration
{
    1: UUID.UUID unitId
    2: UUID.UUID recordingId;
    3: string campaign;
    4: string tag;
/** OBSOLETE_FIELD
    5: string measurementNumber;
**/
    6: string description;
    7: string location;
    8: string fileName;
    9: RecordingType recordingType;
    /** Zero here implies no duration - this could be made optional? */
    10: i64 recordingDuration;
    11: i64 POSIXStartTime;
    12: i32 measurementNumber;
}

struct Status
{
    1: UUID.UUID unitId;
    2: bool isRecording;
    /** Zero indicates no prior recording, 
     *  If recordingState is true AND there is a recordingId THEN this field
     *  is the current recording id 
     *  If recordingState is false AND there is a recordingId, THEN this field
     *  is the previous recording id
     */
    3: UUID.UUID recordingId;
    4: double hardDriveUsedPercent;
    5: double batteryRemainingPercent;
    /** OBSOLETE_FIELDS (Preserved here so we don't re-use the field ID)
    6: string systemTime
    7: optional double latitude; // See #6
    8: optional double longitude; // See #6
    9: optional double altitude; // See #6
    **/
    10: i64 systemTime;

    /** 
     * URI of the CVRS if connected.
     */
    11: optional string cvrsURI;

    /** 
     * Current session id with the CVRS.  
     * No cvrsSession indicates that there is no active session connecting to the CVRS 
     */
    12: optional UUID.UUID cvrsSessionId;
}

struct DataFilter
{
    /** Empty list specifies that PTU MUST NOT send component data */
    1: list<UUID.UUID> componentIds;
    2: bool includeMessages = true;
    3: bool includeWaypoints = true;
    4: bool includeBoundingBoxes = true;
    5: bool includeMarkers = true;
}

struct UnitDefinition
{
    1: UUID.UUID unitId;
    2: string unitName;
    3: string softwareVersion;
    5: string hardwareRevision;
    6: string vendor;
    7: UnitType unitType;
    8: string protocolVersion = PROTOCOL_VERSION;
}

struct SystemConfiguration
{
    1: UUID.UUID unitId;
    2: optional list<GammaSensor.GammaListAndSpectrumConfiguration> gammaSpectrumConfigurations;
    3: optional list<GammaSensor.GammaListAndSpectrumConfiguration> gammaListConfigurations;
    4: optional list<GammaSensor.GammaGrossCountConfiguration> gammaGrossCountConfigurations;
    5: optional list<GammaSensor.GammaDoseConfiguration> gammaDoseConfigurations;
    // 6: optional list<NeutronSensor.NeutronConfiguration> neutronListConfigurations;
    // 7: optional list<NeutronSensor.NeutronConfiguration> neutronSpectrumConfigurations;
    8: optional list<NeutronSensor.NeutronConfiguration> neutronGrossCountConfigurations;
    9: optional list<ContextSensor.ContextVideoConfiguration> contextVideoConfigurations;
    10: optional list<ContextSensor.Context3DConfiguration> contextPointCloudConfigurations;
    11: optional list<ContextSensor.Context3DConfiguration> contextVoxelConfigurations;
    12: optional list<ContextSensor.Context3DConfiguration> contextMeshConfigurations;
    // 3: optional list<EnvironmentalSensor.EnvironmentalSensorConfiguration> environmentSensorConfigurations;
    // 4: optional list<NavigationSensor.NavigationSensorConfiguration> NavigationSensorConfigurations;
    // Probably one of the first things to consider is whether or not each algorithm configuration should go here instead of one algorithm 
    // payload (similar to the way that sensor data is handled)
    13: optional list<AlgorithmPayload.AlgorithmConfiguration> algorithmConfigurations;
    14: optional list<NeutronSensor.NeutronListAndSpectrumConfiguration> neutronListConfigurations;
    15: optional list<NeutronSensor.NeutronListAndSpectrumConfiguration> neutronSpectrumConfigurations;
    16: optional list<PassiveMaterial.PassiveMaterialConfiguration> passiveMaterialConfigurations; 
    17: i64 timeStamp;
    18: optional list<ContextSensor.ContextStreamConfiguration> contextStreamConfigurations;

}

struct SystemDefinition
{
    1: optional list<GammaSensor.GammaListAndSpectrumDefinition> gammaSpectrumDefinitions;
    2: optional list<GammaSensor.GammaListAndSpectrumDefinition> gammaListDefinitions;
    3: optional list<GammaSensor.GammaGrossCountDefinition> gammaGrossCountDefinitions;
    4: optional list<GammaSensor.GammaDoseDefinition> gammaDoseDefinitions;
    5: optional list<NeutronSensor.NeutronListDefinition> neutronListDefinitions;
    6: optional list<NeutronSensor.NeutronSpectrumDefinition> neutronSpectrumDefinitions;
    7: optional list<NeutronSensor.NeutronGrossCountDefinition> neutronGrossCountDefinitions;
    8: optional list<EnvironmentalSensor.EnvironmentalSensorDefinition> environmentalDefinitions;
    9: optional list<NavigationSensor.NavigationSensorDefinition> navigationSensorDefinitions;
    10: optional list<ContextSensor.ContextVideoDefinition> contextVideoDefinitions;
    11: optional list<ContextSensor.Context3DDefinition> contextPointCloudDefinitions;
    12: optional list<ContextSensor.Context3DDefinition> contextVoxelDefinitions;
    13: optional list<ContextSensor.Context3DDefinition> contextMeshDefinitions;
    // Probably one of the first things to consider is whether or not each algorithm definition should go here instead of one algorithm 
    // payload (similar to the way that sensor data is handled)
    14: optional list<AlgorithmPayload.AlgorithmDefinition> algorithmDefinitions;
    15: string apiVersion = PROTOCOL_VERSION;
    16: optional list<PassiveMaterial.PassiveMaterialConfiguration> passiveMaterialDefinitions;
    17: i64 timeStamp;
    18: optional list<ContextSensor.ContextStreamDefinition> contextStreamDefinitions;
}

/**
 *  The data payload is designed to divide the data into 100 ms sections.  The method for dividing the data is to conform to the 
 *  following rules:
 *
 *  - The timestamp below indicates the END of the time period in question.  Therefore, the data included in the packet is data
 *    that was generated between the previous timestamp and this timestamp.
 *  - When possible, this data payload SHOULD include ALL of the data for the 100 ms time slice in question.
 *  - If new data comes in from a sensor for a time period that has already been sent, then it may be sent in a packet with
 *    the correct timestamp by itself (do not resend all of the data for the time period that has already been sent as this would
 *    result in duplicating data in the data stream).  HOWEVER, do not expect algorithms or the CVRS to be able to make
 *    use of this data (although they could), it is only included for completeness of the data set.
 */
struct DataPayload
{
    1: UUID.UUID unitId;
    /** Milliseconds since Unix epoch.  Indicates the END of the time period for which the data is relevant */
    2: i64 timeStamp;
    3: Health.Health systemHealth;
    /** Sent in *emtpy* message with this set to true when the data stream is over for a recording */
    4: bool isEOF = false;
    /** Only sent with very first message or if the configuration changes (i.e. duration changes)*/
    5: optional RecordingConfiguration recordingConfig;
    /** Gamma data associated with this timestamp */
    6: optional list<GammaSensor.GammaSpectrumData> gammaSpectrumData;
    7: optional list<GammaSensor.GammaListData> gammaListData;
    8: optional list<GammaSensor.GammaGrossCountData> gammaGrossCountData;
    9: optional list<GammaSensor.GammaDoseData> gammaDoseData;
    /** Neutron data associated with this timestamp */
    10: optional list<NeutronSensor.NeutronListData> neutronListData;
    11: optional list<NeutronSensor.NeutronSpectrumData> neutronSpectrumData;
    12: optional list<NeutronSensor.NeutronGrossCountData> neutronGrossCountData;
    /** Environmental data associated with this timestamp*/
    13: optional list<EnvironmentalSensor.EnvironmentalSensorData> environmentalData;
    /** Navigation data associated with this timestamp */
    14: optional list<NavigationSensor.NavigationData> navigationData;
    /** Context data associated with this timestamp */
    15: optional list<ContextSensor.ContextVideoData> videoData;
    16: optional list<ContextSensor.ContextPointCloudData> pointCloudData;
    17: optional list<ContextSensor.ContextVoxelData> voxelData;
    18: optional list<ContextSensor.ContextMeshData> meshData;
    /** Messages associated with this timestamp */
    19: optional list<WINDMessage.Message> messages;
    /** Waypoints associated with this timestamp */
    20: optional list<Navigation.Waypoint> waypoints;
    /** Bounding boxes associated with this timestamp */
    21: optional list<Navigation.BoundingBox> boundingBoxes;
    /** Markers associated with this timestamp */
    22: optional list<Marker.Marker> markers;
    // Probably one of the first things to consider is whether or not each algorithm result should go here instead of one algorithm 
    // payload (similar to the way that sensor data is handled)
    23: optional list<AlgorithmPayload.AlgorithmData> algorithmData;
    24: optional list<ContextSensor.ContextStreamIndexData> streamIndexData;

    /** Updates to the configuration.  
     * This is included whenever a configuration is changed.
     * This should be an incremental update containing only the configuration elements that were altered.
     * Changes in a configuration value apply to the current payload data and future payloads.
     */
    25: optional list<SystemConfiguration> configuration;
}
