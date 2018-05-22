namespace cpp wind
namespace java org.wind
namespace csharp org.wind

/**
 * UUIDs in this system MUST behave in the following manner:
 *  * When used as a component id, UUID MUST be the same for each data payload and SHOULD
 *    be the same every time the system restarts EXCEPT WHEN the component itself is changed
 *    (e.g. a detector is replaced with a new one)
 *  * When used as a algorithm id, UUID MUST be the same for each data payload and SHOULD
 *    be the same every time the system restarts EXCEPT WHEN the algorithm code is changed
 *  * When used as a unit id, UUID MUST be the same for each message and SHOULD be the
 *    same every time the system restarts EXCEPT WHEN the definition has changed
 *  * When used as a message, bounding box, and marker id, the UUID SHOULD be random
 *
 *  When generating a UUID that is to be constant then UUID version 3 or 5 MUST be used.
 *  When generating a UUID that is to be random, then UUID version 4 MUST be used.
 **/ 
struct UUID
{
    1: i64 mostSignificantBits;
    2: i64 leastSignificantBits;
}