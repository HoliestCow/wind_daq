namespace cpp wind
namespace java org.wind
namespace csharp org.wind

struct EnergyCalibration
{
	1:  i32 channel;
	/** KeV */
	2:  double energy;
}

struct EnergyResolution
 {
 	/** KeV */
 	1:  double energy;
 	/** (full-width-half-height energy width in KeV) / (center energy KeV) */
 	2:  double fraction;
 }

enum DetectorMaterial
{
	HPGe,
	HPXe,
	NaI,
	LaBr3,
	LaCl3,
	BGO,
	CZT,
	CdTe,
	CsI,
	GMT,
	GMTW,
	LiFiber,
	PVT,
	PS,
	He3,
	He4,
	LiGlass,
	LiI,
	SrI2,
	CLYC,
	CdWO4,
	BF3,
	HgI2,
	CeBr4,
	LiCAF,
	LiZnS,
	Other
}
