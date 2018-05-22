namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "Health.thrift"
include "DetectorCharacteristics.thrift"
include "Spectrum.thrift"
include "Angular.thrift"
include "UUID.thrift"
include "PhysicalDimensions.thrift"
include "Common.thrift"
include "Component.thrift"

/*
 * Defines sensor settings that are changeble, but aren't data gathered by the sensor
 */
struct NeutronConfiguration
{
	1: UUID.UUID componentId;  // Used to match data and configuration, must be unique in the system
	2: Component.GridPositionAndOrientation componentPositionAndOrientation;
}

struct NeutronListAndSpectrumConfiguration
{
	1: UUID.UUID componentId;  // Used to match data and configuration, must be unique in the system
	2: Component.GridPositionAndOrientation componentPositionAndOrientation;
	3: list<DetectorCharacteristics.EnergyCalibration> energyCalibration;
}

/*
 * Defines what the sensor is and what its capabilities are.  This should not change during normal operation
 */
struct NeutronListDefinition
{
	1: Component.ComponentDefinition component;
	5: i32 numberOfChannels;
	6: PhysicalDimensions.Dimensions physicalDimensions; 
	7: DetectorCharacteristics.DetectorMaterial detectorMaterial;
	// 8: NeutronConfiguration startingListNeutronConfiguration;
	9: NeutronListAndSpectrumConfiguration startingListNeutronConfiguration;
	10: list<Angular.AngularEfficiencyDefinition> angularEfficiencies;
}

struct NeutronListData
{
	1: UUID.UUID componentId;  // Used to match data and configuration, must be unique in the system
	/** Milliseconds since UNIX epoch */
	2: i64 timeStamp;
	3: Health.Health health;
	4: list<Spectrum.ListMode> listModeData;
	5: i64 liveTime; // milliseconds 
	6: i64 realTime; // milliseconds
	// 7: optional NeutronConfiguration ListNeutronConfiguration // Send this only if things have changed
	//8: optional NeutronListAndSpectrumConfiguration  listNeutronConfiguration; // Send this only if things have changed
}

/*
 * Defines what the sensor is and what its capabilities are.  This should not change during normal operation
 */
struct NeutronSpectrumDefinition
{
	1: Component.ComponentDefinition component;
	5: i32 numberOfChannels;
	6: PhysicalDimensions.Dimensions physicalDimensions; 
	7: DetectorCharacteristics.DetectorMaterial detectorMaterial;
	// 8: NeutronConfiguration startingSpectrumNeutronConfiguration;
	9: NeutronListAndSpectrumConfiguration startingSpectrumNeutronConfiguration;
	10: list<Angular.AngularEfficiencyDefinition> angularEfficiencies;
}

struct NeutronSpectrumData
{
	1: UUID.UUID componentId;  // Used to match data and configuration, must be unique in the system
	/** Milliseconds since UNIX epoch */
	2: i64 timeStamp;
	3: Health.Health health;
	4: Spectrum.SpectrumResult spectrum;
	5: i64 liveTime; // milliseconds 
	6: i64 realTime; // milliseconds
	// 7: optional NeutronConfiguration neutronSpectrumConfiguration // Send this only if things have changed
	// 8: optional NeutronListAndSpectrumConfiguration neutronSpectrumConfiguration; // Send this only if things have changed
}

/*
 * Defines what the sensor is and what its capabilities are.  This should not change during normal operation
 */
struct NeutronGrossCountDefinition
{
	1: Component.ComponentDefinition component;
	5: PhysicalDimensions.Dimensions physicalDimensions;
	6: DetectorCharacteristics.DetectorMaterial detectorMaterial;
	7: NeutronConfiguration startingNeutronConfiguration;
}

struct NeutronGrossCountData
{
	1: UUID.UUID componentId;  // Used to match data and configuration, must be unique in the system
	/** Milliseconds since UNIX epoch */
	2: i64 timeStamp;
	3: Health.Health health;
	4: i32 counts;
	5: i64 liveTime; // milliseconds 
	6: i64 realTime; // milliseconds
	// 7: optional NeutronConfiguration neutronGrossCountConfiguration // Send this only if things have changed
}
