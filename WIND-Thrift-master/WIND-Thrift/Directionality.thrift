namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "Common.thrift"

/**
 * Directionalitys are produced by the Algorithm and returned to the 
 * DAQ for the UI and archiving.
 */

struct DirectionalityConfiguration
{
  /** The time of the rolling window used in computing the
    directionality result.
   */
  1: double integrationTime
}

struct DirectionalityDefinition
{
  1: DirectionalityConfiguration initialConfiguration

  /** Type of directionality result being reported.  
   */
  2: Common.AlgorithmDetectorType type
}

/** Directionality result indicating the direction to a source
  relative to the current position and orientation of the system.  
  This is to support a compass like display for the user such as 
  augmented reality.

  Directionality results must be reported incrementally on regular 
  intervals while a source is in the field of view as defined in
  the algorithm payload.

  The bearing and elevation shall apply the current time interval
  reported. The distance may use information from previous positions
  and thus may use a larger time window.  The longer time window for
  location will not be reported in the AlgorithmResult.
  
  Not every result will be able to provide elevation and distance.
  They will be provided when available, and the same event may 
  produce multiple results for different time periods in which 
  direction and elevation provided for only some of the results.

  An algorithm shall produce no more than one directionality per 
  event id per detector type per time period.  Multiple sources 
  can be reported in the same time with different event ids.

  Directionality results must be associated with an event id.
 */
struct DirectionalityResult
{
  /** Bearing to the source relative to the current orientation of the
       detector system in degrees.
    */
  1: double bearing;

  /** Uncertainty in terms of one standard deviations. */
  2: optional double bearingUncertainty;

  /** Distance in meters to the estimated source position relative
     to the current location of the system if it can be determined.
   */
  3: optional double distance;

  /** Uncertainty in terms of one standard deviations. */
  4: optional double distanceUncertainty;

  /** Elevation of the source in degrees relative to level
     along the bearing if it can be determined.  A positive elevation
     indicates a source above the center of the detector system
      while a negative elevation indicated a source below.
    */
  5: optional double elevation;

  /** Uncertainty in terms of one standard deviations. */
  6: optional double elevationUncertainty;
}
