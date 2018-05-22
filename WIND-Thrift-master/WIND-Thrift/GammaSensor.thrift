namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "Health.thrift"
include "DetectorCharacteristics.thrift"
include "Angular.thrift"
include "Spectrum.thrift"
include "UUID.thrift"
include "PhysicalDimensions.thrift"
include "Common.thrift"
include "Component.thrift"

struct SIPMSettings
{
	1: double highVoltage;
}

struct PMTSettings
{
	1: double highVoltage;
	2: double gain;
	3: double lowerLevelDiscriminator;
	4: double upperLevelDiscriminator;
}

union SIPM_PMTSettings
{
	1: SIPMSettings sipmSettings;
	2: PMTSettings pmtSettings;
}

/*
 * Defines sensor settings that are changeble, but aren't data gathered by the sensor
 */
struct GammaListAndSpectrumConfiguration
{
	/** Used to match data, configuration, and definition.  Must be unique in the system */
	1: UUID.UUID componentId;
	2: SIPM_PMTSettings settings;
	3: list<DetectorCharacteristics.EnergyCalibration> energyCalibration;
	4: Component.GridPositionAndOrientation componentPositionAndOrientation;
	/** Sensor resolution, specified at multiple energy levels. Must have sufficient entries to 
	  * adequately characterize sensor resolution using a log/log interpolation to within 10% 
	  * over the range 0-3000keV.
	*/
	5: list<DetectorCharacteristics.EnergyResolution> energyResolution;
}

/*
 * Defines what the sensor is and what it's capabilities are.  This should not change during normal operation
 */
struct GammaListAndSpectrumDefinition
{
	1: Component.ComponentDefinition component
	5: i32 numberOfChannels;
	6: PhysicalDimensions.Dimensions physicalDimensions; 
	7: DetectorCharacteristics.DetectorMaterial detectorMaterial;
	/** Starting values for the configuration options */
	8: GammaListAndSpectrumConfiguration startingGammaConfiguration;
	// Used for source locating
	9: list<Angular.AngularEfficiencyDefinition> angularEfficiencies;
}

struct GammaSpectrumData
{
	/** Used to match data, configuration, and definition.  Must be unique in the system */
	1: UUID.UUID componentId;
	/** Milliseconds since UNIX epoch */
	2: i64 timeStamp;
	3: Health.Health health;
	4: Spectrum.SpectrumResult spectrum;
	/** Send this only if things have changed */
	// 5: optional GammaListAndSpectrumConfiguration gammaSpectrumConfiguration;
	6: i64 liveTime; // milliseconds 
	7: i64 realTime; // milliseconds
}

struct GammaListData
{
	/** Used to match data, configuration, and definition.  Must be unique in the system */
	1: UUID.UUID componentId;
	/** Milliseconds since UNIX epoch */
	2: i64 timeStamp;
	3: Health.Health health;
	/** Send this only if things have changed */
	// 4: optional GammaListAndSpectrumConfiguration gammaListConfiguration;
	5: list<Spectrum.ListMode> listModeData;
	6: i64 liveTime; // milliseconds 
	7: i64 realTime; // milliseconds
}

/*
 * Defines sensor settings that are changeble, but aren't data gathered by the sensor
 */
struct GammaGrossCountConfiguration
{
	/** Used to match data, configuration, and definition.  Must be unique in the system */
	1: UUID.UUID componentId;
	2: Component.GridPositionAndOrientation componentPositionAndOrientation;
}

/*
 * Defines what the sensor is and what it's capabilities are.  This should not change during normal operation
 */
struct GammaGrossCountDefinition
{
	1: Component.ComponentDefinition component
	5: PhysicalDimensions.Dimensions physicalDimensions; 
	6: DetectorCharacteristics.DetectorMaterial detectorMaterial;
	7: GammaGrossCountConfiguration startingGammaGrossCountConfiguration; // Starting values for the configuration options
	8: list<Angular.AngularEfficiencyDefinition> angularEfficiencies;
}

struct GammaGrossCountData
{
	/** Used to match data, configuration, and definition.  Must be unique in the system */
	1: UUID.UUID componentId;
	/** Milliseconds since UNIX epoch */
	2: i64 timeStamp;
	3: Health.Health health;
	4: i32 counts
	5: i64 liveTime; // milliseconds 
	6: i64 realTime; // milliseconds
}

/*
 * Defines sensor settings that are changeble, but aren't data gathered by the sensor
 */
struct GammaDoseConfiguration
{
	/** Used to match data, configuration, and definition.  Must be unique in the system */
	1: UUID.UUID componentId;
	2: Component.GridPositionAndOrientation componentPositionAndOrientation;
}

/*
 * Defines what the sensor is and what it's capabilities are.  This should not change during normal operation
 */
struct GammaDoseDefinition
{
	1: Component.ComponentDefinition component;
	5: PhysicalDimensions.Dimensions physicalDimensions; 
	6: DetectorCharacteristics.DetectorMaterial detectorMaterial;
	7: GammaDoseConfiguration startingGammaDoseConfiguration; // Starting values for the configuration options
	8: list<Angular.AngularEfficiencyDefinition> angularEfficiencies;
}

struct GammaDoseData
{
	/** Used to match data, configuration, and definition.  Must be unique in the system */
	1: UUID.UUID componentId;
	/** Milliseconds since UNIX epoch */
	2: i64 timeStamp;
	3: Health.Health health;
	4: double dose;
	5: i64 liveTime; // milliseconds 
	6: i64 realTime; // milliseconds
}
