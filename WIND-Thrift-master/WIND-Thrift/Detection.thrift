namespace cpp wind
namespace java org.wind
namespace csharp org.wind

/** Detection metrics are provided by the algorithm. */

include "Identification.thrift"
include "Spectrum.thrift"


/**
 * Configuration
 */
struct DetectionConfiguration
{
    /** List of nuclides this algorithm can detect */
    1: optional list<Identification.Nuclide> nuclidesSensitivity
    /** Time in milliseconds of background background window */
    2: i64 backgroundWindowSize
    /** Time in milliseconds of foreground window */
    3: optional i64 foregroundWindowSize
    /** Scalar */
    4: optional double lowerAlarmThreshold
    /** Scalar */
    5: optional double upperAlarmThreshold
    /** KeV */
    6: optional double lowerROIThreshold
    /** KeV */
    7: optional double upperROIThreshold
}

/**
 * Definition
 */
struct DetectionDefinition
{
  1: DetectionConfiguration startingDetectionConfiguration;
}

/**
 * Result
 */
struct DetectionResult
{
    /* Figure of merit returned by detection algorithm.

       The figure of merit shall be scaled to the significance of the finding
       relative to background under the operating conditions.

       The significance is expressed in terms of frequency of occurance on a
       logrithmic scale.  A metric of 1.0 shall indicate that the probability
       of this occuring without the presence of a source would occur no more
       often than once every 15 minutes.  A metric of 3.0 shall indicate that
       the expected decision shall not occur more than once every hour.

       The figure of metric shall be on a continuous scale such that a
       stronger hit for the same source will always produce a larger metric.
       Different sources may have different efficiencies thus the source
       intensity does not have to produce the same metric.  For example,
       sources that are dissimilar to background (ie. Am241) will likely
       produce stronger detection metric than sources that are similar
       background or cosmic continuum (Ie. Sr90).

       An algorithm may produce multiple decision metrics.  When multiple
       decision metrics are calculated, the metrics must include the false
       alarm rate of all metrics in computing the scale.  Thus if multiple
       metrics were computed, no more than one of the metrics shall be
       greater than 1.0 during a 15 minute period when no source is present.

       For example, an algorithm could report gamma decisions and neutron
       decision seperately but must account for both when reporting the
       significance scale of each.

       Decision metrics shall be in the same order and uniquely named in the
       definition.

     */
    1: double decisionMetric;

    /* Removed: range of time covered in algorithm header.
    2: i64 timeOfDecision 
    */

    3: optional Spectrum.SpectrumResult resultantSpectrum
    4: bool inAlarm // Simple yes/no telling other algorithms if the system is in alarm

    /**
     * This is the total gamma dose estimated at center.  (origin of the detector system)
     *
     * This is not the sum of the dose on each detector.
     */
    5: optional double gammaDose

    /**
     * This is the total neutron dose estimated at center.  (origin of the detector system)
     *
     * This is not the sum of the dose on each detector.  May be based on the neutron and gamma data.
     */
    6: optional double neutronDose
}
