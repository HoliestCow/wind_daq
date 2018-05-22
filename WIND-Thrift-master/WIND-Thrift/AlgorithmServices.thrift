namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "PTUPayload.thrift"
include "AlgorithmPayload.thrift"
include "Exceptions.thrift"
include "Command.thrift"
include "UUID.thrift"

service Algorithm
{
    /**
     * Defines what components and algorithms the PTU has available for subscribing to.
     *
     * This function MUST be called every time the PTU changes (i.e. a new algorithm registers with the PTU)
     *
     * Returns EITHER a list of components that the algorithm would like to subscribe to OR an empty list.
     *  -If an empty list is returned, then the algorithm subscribes to all data in the system.
     *  -If this function is called again by the PTU then the algorithm MUST return a new
     *   list that replaces the previous subscriptions (note this is not additive!)
     *  -Algorithms MAY subscribe to other algorithms.  Algorithms will receive the output from
     *   the algorithm that is subscribed to in the next payload.  Thus circular dependencies
     *   do not pose an issue.
     */
    list<UUID.UUID> definePTU(1: PTUPayload.SystemDefinition systemDefinition);

    /**
     * Get the capabilities of the algorithm based on the provided hardware.
     *
     * The algorithm definition is produced in response to definePTU and will produce an empty list
     * if the PTU is not defined.  The algorithm definition shall only change when the PTU is redefined.
     * There is one algorithm definition entry for each data product that is produced by the algorithm.
     */
    list<AlgorithmPayload.AlgorithmDefinition> getAlgorithmDefinition();

    /**
     * Gets how the algorithms are configured, such as background times, etc...
     *
     * Returns current algorithm configurations
     */
    list<AlgorithmPayload.AlgorithmConfiguration> getAlgorithmConfiguration();

    /**
     * Sets algorithm configuration options.
     *
     * Returns updated algorithm configuration
     */
    AlgorithmPayload.AlgorithmConfiguration setAlgorithmConfiguration(1: AlgorithmPayload.AlgorithmConfiguration configuration) throws (1:Exceptions.UpdateError error);

    /**
     * Gets called every 100 ms to update the algorithm on the latest data.
     *
     * The algorithm MUST respond with a systemData packet every 100 ms, but the list may be empty if there are no computed results.
     */
    list<AlgorithmPayload.AlgorithmData> updateAlgorithm(1: PTUPayload.DataPayload systemData);

    /**
     * Gives the PTU (and thus the PTU UI and CVRS) the ability to send a command to the algorithm
     * (such as reset background).
     *
     * MAY return either the command's response or an empty string
     */
    string sendCommand(1: Command.Command command);
}
