namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "Identification.thrift"
include "Localization.thrift"
include "Directionality.thrift"
include "Detection.thrift"
include "Health.thrift"
include "UUID.thrift"
include "ProcessedImage.thrift"
include "Common.thrift"
include "Component.thrift"
include "NavigationSensor.thrift"


/** Indication of how to interpret log messages.  Severity of WARNING
 * and above indicate a fault that may adversely affect the
 * algorithm output.
 */
enum LogSeverity
{
  /** Error from which no further results can be produced without reinitialization.
   */
  FATAL,

  /** Error preventing interpretation of the data stream.  This
   * condition may clear in future packets.
   */
  ERROR,

  /** Error adversely affecting the results of the algorithm, but
   * but for which the results are usable but degraded.
   */
  WARNING,

  /** Logging message indicating how data has been interpreted.
   * These should only be issued for AlgorithmDefinition
   * response.
   */
  INFO,

  /** Indicates that a condition is no longer applicable.
   */
  RESOLVED
}

/** Data structure to push messages to the UI and archiver on the
 * state of an algorithm.
 *
 * These should be used sparingly to indicate details on the state
 * of health of an algorithm.  Log messages are expected to be issued
 * after receiving the system definition, a new configuration, or
 * when there is a significant change that adversely affects
 * the results of the algorithm.
 */
struct AlgorithmLog
{
  /** A unique id associated with this fault either issued
   * as a fixed value for the same fault used consistently
   * to report the same error condition or a unique id
   * for a dynamically issued fault.
   *
   * Faults are assumed to be stateful.  Thus if a ERROR is
   * issued, the error state is assumed to persist until such
   * time as it is cleared.  To indicate that a condition is
   * no longer present an END severity is issued withe same
   * referenceId.
   *
   * There is no requirement terminate a condition, but the data
   * will be interpreted as though the condition is persistent.
   */
  1: UUID.UUID referenceId;

  /** Severity of the fault.
   */
  2: LogSeverity severity;

  /** Human readable message describing the fault.  All log messages
   * except for those of type END shall have a message content.
   */
  3: optional string message;

  /** Component id of the sensor which caused the fault (if applicable)
   */
  4: optional UUID.UUID reference;
}


union AlgorithmConfigurationUnion
{
    1: Detection.DetectionConfiguration detectionConfiguration;
    2: Identification.IdentificationConfiguration identificationConfiguration;
    3: Localization.LocalizationConfiguration localizationConfiguration;
    4: Directionality.DirectionalityConfiguration directionalityConfiguration;
    5: ProcessedImage.ProcessedImageConfiguration processedImageConfiguration;
    // Definitions for new algorithm configurations will be placed here
}

/**
 * Defines algorithm settings that are changeable, but aren't data gathered by the sensor
 */
struct AlgorithmConfiguration
{
    /** Used to match data, configuration, and definition.  Must be unique in the system. */
    1: UUID.UUID componentId;

    /** Polymorphic Data message */
    2: AlgorithmConfigurationUnion contents

    /** Messages produced in response to the last Configuration request.
     * This will used be indict how the algorithm is interpreting
     * the configuration change.
     *
     * This may contain any type of log message. Faults of ERROR or FATAL
     * will indicate that the system is not functional under the current
     * system configuration.  Faults of level WARNING indicate conditions
     * that can be tolerated but may affect performance.  INFO is used
     * to indicate any other applicable data.
     */
    3: optional list<AlgorithmLog> log;
}

/**
 * Defines a specific product for an algorithm.
 *
 * The algorithm definition is produced in response to the PTU definition based on the capabilities of the
 * algorithm given the specified hardware.  Algorithms can produce multiple different algorithm products
 * and may produce multiple implementation of the same data product.
 *
 * Each algorithm product should have its own unique componentId within the system.
 * That componentId will be used to differentiate if more than one data product of the
 * same type is produced by the algorithm.
 */
union AlgorithmDefinitionUnion
{
    1: Detection.DetectionDefinition detectionDefinition;
    2: Identification.IdentificationDefinition identificationDefinition;
    3: Localization.LocalizationDefinition localizationDefinition;
    4: Directionality.DirectionalityDefinition directionalityDefinition;
    5: ProcessedImage.ProcessedImageDefinition processedImageDefinition;
    6: NavigationSensor.NavigationOutputDefinition navOutputDefinition;
    // Definitions for new algorithm products will be placed here
}

struct AlgorithmDefinition
{
    1: Component.ComponentDefinition component;

    /** Polymorphic data type. */
    5: AlgorithmDefinitionUnion contents;

    /** Messages produced in response to the last SystemDefinition.
     * This will used be indict how the algorithm is interpreting
     * the system.
     *
     * This may contain any type of log message. Faults of ERROR or FATAL
     * will indicate that the system is not functional under the current
     * system definition.  Faults of level WARNING indicate conditions
     * that can be tolerated but may affect performance.  INFO is used
     * to indicate any other applicable data.
     */
    6: optional list<AlgorithmLog> log;
}

/**
* Algorithm debug result structure.  This should not be used for normal operation.
*/
struct AlgorithmDebug
{
  1: map<string, binary> debugData;
}

/**
 * Individual data for an analysis result.
 *
 * This structure contains common header information related to all results and a union
 * of different data types.  The data structure hold the results from any of the algorithm
 * components.
 *
 * Each algorithm component may produce, at most, one report for each source in view.
 *
 * Algorithm components should produce, at most, one type of data.  These may be
 * detection, localization, or identification.  If a particular algorithm
 * component has more than one type of result, the result must have a separate component
 * id and definition.  This allows the stream to be filtered based on the algorithm
 * and data type.
 *
 * Each time a source is encountered, a new event id will be issued by the system.
 * The event id must be random as specified in the UUID documentation.
 *
 * All algorithm products related to that source will have the same event id.
 * Products associated with a source must use that event id until the source leaves
 * the field of view.  For algorithms that are performing a post analysis of the output of
 * another algorithm will use the same time range and event id.  More than one source
 * may be in the field of view at a time.
 *
 * Each data product has different requirements with regard to association with events,
 * timestamp reporting, and production periods.  These requirements include
 *
 *  - Regular intervals - Each result should be produced on a regular interval. The start time
 *    and end time should reflect the earlier.
 *
 *  - As-available - The result should be updated whenever there is sufficient
 *    information available to improve the result last reported.  As-available should
 *    produce a final result after the source leaves the field of view.
 *
 *  - Incremental reporting - The result must should be for a specific period of time and
 *    must cover the range of measurements which were added since the last report.  Results
 *    should never cover the same range.  Prior information may have been incorporated in forming
 *    the report but should not reported in the time range.
 *
 *  - Cumulative reporting - The result may cover a range of measurement which may include
 *    the previous time period.  The start and end time should cover the earliest and latest
 *    measurements that were incorporated. Not all measurements during that time interval
 *    may be been used in producing a result.  Cumulative reports may cover the same
 *    time range as previous reports if the previous data was still applicable.  If the
 *    previous conditions no longer applied, the start time should reflect the time the source
 *    was in those conditions,  i.e, if a source appears to be stationary the start time will
 *    cover the entire time the source was present and the end time will be revised with each
 *    sequential result.  If the source is mobile, the start time and the end time
 *    will be updated with each result such that the result covers the time period that source
 *    was at that location.
 *
 *
 * Detection results should not be associated with an event as there may be more that one source in the
 * field of view, i.e., AlgorithmData.eventId should not be set.
 * Detection algorithms must produce a result whenever a source is in view.  Detection results must be
 * produced at a regular intervals and incrementally. Detection algorithms must have an a configuration
 * option to produce results even when a source is not present.  There should be no more that one detection result from a
 * detection component for the same time range.
 *
 * Localization results must be associated with an event.  Localization should be as-available and
 * cumulative.
 *
 * Identification results must be associated with an event. Identification
 * results should be reported as-available and may be cumulative or incremental.
 *
 * ProcessedImage results may or may not be associated with an event. An algorithm may produce map or image overlays
 * that are associated with an event to help localize a detected source. One can imagine, however, images being produced
 * that simply lead to an improved map of an area.
 *
 * NavigationData PTU localization results should not be associated with an event. An algorithm may produce improved 
 * localization of a PTU that is useful even without source detection, localization, and identification occurring.
 *
 */

union AlgorithmUnion
{
  1: Detection.DetectionResult detectionResult
  2: Identification.IdentificationResult identificationResult
  3: Localization.LocalizationResult localizationResult
  4: Directionality.DirectionalityResult directionalityResult
  5: ProcessedImage.ProcessedImageResult processedImageResult
  6: NavigationSensor.NavigationData ptuLocalizationResult
  // Definitions for new algorithm results will be placed here
}

/**
 * Algorithm Server state
 * Note that the client (PTU or CVRS) should have a reasonable timeout on waiting for
 * algorithm results.
 */

enum AlgorithmServerState
{
  /* No algorithm results will be forthcoming without additional sensor data */
  Idle = 1,
  /* Algorithm results are being computed and will be returned at a future time */
  Busy = 2
}

struct AlgorithmData
{
  /** The algorithm that is reporting this result. */
  1: UUID.UUID componentId;

  /** Identifier to link multiple analyses together for one source.
   *
   * This must be set for any algorithm results that are tied to a specified source encounter.
   * The same event number should be reported during the time a source is visible.  If
   * a second source is encountered while a source is visible, it should be associated with
   * another event id.
   *
   * eventId should not be set for detection results
   */
  2: optional UUID.UUID eventId;

  /** Time range of that this analysis covers */
  3: i64 startTimeStamp;
  4: i64 endTimeStamp;

  5: Health.Health health;

  /** Polymorphic Data message */
  6: optional AlgorithmUnion contents;

  /** Debug result */
  7: optional AlgorithmDebug debugResult;

  /** Algorithm Server status */
  8: AlgorithmServerState  serverState;

  /** Details on the state of the algorithm to indict a fault.
   *
   * Faults should be issued sparingly and are considered stateful.
   * A state is assumed to presist until such time as the fault
   * state is cleared with an END message. INFO type
   * messages are not allowed in the AlgorithmData.
   */
  9: optional list<AlgorithmLog> faults;
}




