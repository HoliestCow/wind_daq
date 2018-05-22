namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "Component.thrift"
include "UUID.thrift"
include "PhysicalDimensions.thrift"


struct Constituent 
{
   /* Z should be required, but leaving as optional for reverse compatibility.*/
   1: optional i32 Z; //atomic #
   2: optional i32 MassNumber; //e.g., 6 for 6-Li.  If absent, assume natural enrichment for Z
   3: optional string ConstituentName;
   4: optional double AtomicFraction;
   5: optional double MassFraction;
   /* if neither AtomFraction nor MassFraction specified, assume list has equal atomic percentage e.g., NaCl*/
}
struct Material 
{
   /* grams per cubic centimeter */
   1: optional double Density; 
   2: optional list<Constituent> constituents;
   3: optional string MaterialName;
}
struct PassiveMaterialConfiguration
{
/* Passive materials that are likely to be located between a radiation detector and potential source locations (i.e., everything at and near the horizontal height(s) of the radiation detectors) that can present an areal density of up to 1/4 of a mean free path for a 186 keV photon should be included in a list of passive materials.  This would imply that an object containing water (or water-equivalent materials) that a photon can traverse a 1.75 cm-long path prior to reaching a detector should be included.*/
        1: UUID.UUID componentId;
        2: string componentName; //doesn't need to be unique
        3: string vendorName; //someone to blame for bad definitions
        4: PhysicalDimensions.Dimensions physicalDimensions;
        5: Material passiveMaterial;
        6: Component.GridPositionAndOrientation passiveMaterialPositionAndOrientation;
}
