namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "UUID.thrift"

exception RecordingError
{
	1: string message;
	2: optional string stackTraceString;
}

exception UpdateError
{
	1: string message;
	2: optional string stackTraceString;
}

exception RetrievalError
{
	1: string message;
	2: optional string stackTraceString;
}

/**
 * Exception thrown by PTU service method initiateCVRSSession if the PTU fails to successfully 
 * connect to the CVRS, register itself, and establish a valid session.
 */
exception CVRSRegistrationError
{
	1: string message;
	2: optional string stackTraceString;
}

/**	A union that allows a PTU to send any of the existing exceptions with each ControlMessageAck 
 *	when it fails to execute a command.
 */
union ExceptionUnion {
	1: RecordingError recordingError;
	2: UpdateError updateError;
	3: RetrievalError retrievalError;
}

/**	An InvalidSession exception can be thrown by any of the methods used by the PTU to communicate 
 *	with the CVRS if the session is invalid.
 */
exception InvalidSession {
	1: UUID.UUID sessionId;
	2: string message;
}