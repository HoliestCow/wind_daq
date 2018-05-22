namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "Health.thrift"
include "UUID.thrift"
include "Common.thrift"
include "Component.thrift"

enum EnvironmentalTypes
{
	/** Percent */
	RelativeHumidity,
	/** Pascals */
	Pressure,
	/** Celcius */
	Temperature,
	/** Degrees */
	WindDirection,
	/** Meters/Second */
	WindSpeed,
	UVIndex
}

/**
 * Defines what the sensor is and what it's capabilities are.  This should not change during normal operation
 */
struct EnvironmentalSensorDefinition
{
	1: Component.ComponentDefinition component;
	5: EnvironmentalTypes sensorType;
	6: string location; // I feel like this should probably be more defined
	7: Component.GridPositionAndOrientation componentPositionAndOrientation;
}

struct EnvironmentalSensorData
{
	1: UUID.UUID componentId
	/** Milliseconds since Unix epoch */
	2: i64 timeStamp
	3: Health.Health health; 
	4: double value
}
