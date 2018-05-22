namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "Spectrum.thrift"
include "Navigation.thrift"
include "Common.thrift"

/**
 * Localizations are produced by the Algorithm and returned to the 
 * DAQ for the UI and archiving.
 */

struct LocalizationConfiguration
{
  /** Placeholder for actual configuration */
  1:  bool placeholder;
}

// Do we need separate localization and direction capabilities for
// neutrons and gammas?
struct LocalizationDefinition
{
  1: LocalizationConfiguration initialConfiguration

  /** Type of directionality result being reported.  
   */
  2: Common.AlgorithmDetectorType type
}

/** Localization results indicate the position of a source over 
  a period of time. This is to support placing the estimated
  source position on a map display.

  Localization results are reported as available whenever the 
  algorithm has sufficient information to update the position.

  Localization results are assumed to be cummulative over the 
  period of time that the source was in view.  The time period 
  reported for the localization results shall be the period of
  time that source has been present at that location.  For a
  stationary source this would be the entire encounter duration and
  thus the startTimestamp would be fixed.  For a moving source, 
  the starting time would advance to reflect the earliest data 
  which reflects this new position.  As the estimate of the 
  location may used data which was previously used for a 
  previous estimate, the timesamples between a previous report
  and the revised report may have overlapping timestamps.

  Localization results must be associated with an event id.
  At most one localization result may be produced for each 
  event id per localization algorithm. 

  If algorithms report gamma and neutron localization as 
  independent results, they will define a separate localization 
  algorithm for each sensor type.

  Localization results may report an uncertainty in the source 
  position.  Each dimension is assumed to have an independent 
  uncertainty. 

  FIXME do we need to define the type of localization result
  if the algorithm is of type Gamma/Neutron?  This could be
  implied from the detection result, but it is not clear in 
  detection whether it is a gamma or neutron source. 

  FIXME we need a new type to define uncertainties about a 
  position.  Our current system defines uncertainty along the 
  axis and thus does not adequately describe an elipsode 
  with an axis whose major axis is not along the coordinate axis.

 */
struct LocalizationResult
{ 
  /** The estimated location of the source for the starting to 
      ending timestamp reported.  The source is assumed to be
      at that position until a revised localization result is 
      produced.
   */
  1: Navigation.Location location;

  /** Uncertainty of location of the source in terms of a one standard 
    deviation in each direction as an ellipsoid.
   */
  2: optional Navigation.LocationUncertainty uncertainty;
}

