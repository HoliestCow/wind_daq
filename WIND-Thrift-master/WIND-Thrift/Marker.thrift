namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "GammaSensor.thrift"
include "NeutronSensor.thrift"
include "EnvironmentalSensor.thrift"
include "AlgorithmPayload.thrift"
include "UUID.thrift"
include "Navigation.thrift"

enum MarkerType
{
	PTU,
	Source,
	Other
}

struct Marker
{
	1: UUID.UUID markerId;
	/** POSIX time * 1000 - should match data packet's time stamp */
	2: i64 timeStamp;
	3: string name;
	4: MarkerType type;

/* FIXME should this be renumbered
	5: double latitude;
	6: double longitude;
	7: double altitude;
*/
	5: Navigation.Location location

	/** Unique ID of PTU to be used when type is PTU */
	8: optional UUID.UUID unitId;

	/** Optional data that can be associated with marker and used for generating radiation maps, etc. */
	9: optional list<GammaSensor.GammaSpectrumData> gammaSpectrumData;
    10: optional list<GammaSensor.GammaListData> gammaListData;
    11: optional list<GammaSensor.GammaGrossCountData> gammaGrossCountData;
    12: optional list<GammaSensor.GammaDoseData> gammaDoseData;
    13: optional list<NeutronSensor.NeutronListData> neutronListData;
    14: optional list<NeutronSensor.NeutronSpectrumData> neutronSpectrumData;
    15: optional list<NeutronSensor.NeutronGrossCountData> neutronGrossCountData;
	16: optional list<EnvironmentalSensor.EnvironmentalSensorData> environmentalData;
	17: optional list<AlgorithmPayload.AlgorithmData> algorithmData;

}
