namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "PTUPayload.thrift"
include "UUID.thrift"
include "Navigation.thrift"
include "WINDMessage.thrift"
include "Marker.thrift"
include "Exceptions.thrift"

enum StatusCode {
	OK,
	PROTOCOL_MISMATCH
}

struct Session {
	1:StatusCode status;
	2:optional UUID.UUID sessionId;
}

/** Defines the types of commands that can be sent to the PTU. The payload that must accompany 
 *  each type of control message is defined in the contract below.
 */
enum ControlType {
	/* Request that a recording be started. The ControlMessage.payload MUST be
	 * a StartRecordingControlPayload that contains the same parameters sent to the 
	 * startRecording method of the PTU service.
	 *
	 * If a PTU already has an active recording and receives this message, the
	 * PTU MUST stop the current recording and start a new one.
	 */
	START_RECORDING,
	/* Request that any current recording be stopped. The ControlMessage.payload MUST
	 * be null and the PTU MUST stop recording.
	 */
	END_RECORDING,
	/* Request that a waypoint be updated by the PTU. The waypoint MUST be added if it does not 
	 * already exist, otherwise it MUST be updated. The ControlMessage.payload MUST be 
	 * a Navigation.Waypoint.
	 */
	UPDATE_WAYPOINT,
	/* Request that a waypoint be removed by the PTU. The ControlMessage.payload MUST be 
	 * the UUID.UUID of the waypoint to be removed and the PTU MUST remove it.
	 */
	DELETE_WAYPOINT,
	/* Request that PTU clear all waypoints. The ControlMessage.payload MUST be 
	 * null and the PTU MUST clear its waypoints.
	 */
	CLEAR_ALL_WAYPOINTS,
	/* Request that a bounding box be updated by the PTU. The bounding box MUST be added 
	 * if it does not already exist, otherwise it MUST be updated. The ControlMessage.payload 
	 * MUST be a Navigation.BoundingBox.
	 */
	UPDATE_BOUNDING_BOX,
	/* Request that a bounding box be removed by the PTU. The ControlMessage.payload MUST be 
	 * the UUID.UUID of the bounding box to be removed and the PTU MUST remove it.
	 */
	DELETE_BOUNDING_BOX,
	/* Request that PTU clear all bounding boxes. ControlMessage.payload MUST be 
	 * null and the PTU MUST clear its bounding boxes.
	 */
	CLEAR_ALL_BOUNDING_BOXES,
	/* Request that a message be updated by the PTU. The message MUST be added 
	 * if it does not already exist, otherwise it MUST be updated. The ControlMessage.payload 
	 * MUST be a WINDMessage.Message.
	 */
	UPDATE_MESSAGE,
	/* Request that a message be removed by the PTU. The ControlMessage.payload MUST be 
	 * the UUID.UUID of the message to be removed and the PTU MUST remove it.
	 */
	DELETE_MESSAGE,
	/* Request that PTU clear all messages. ControlMessage.payload MUST be 
	 * null.
	 */
	CLEAR_ALL_MESSAGES,
	/* Request that a marker be updated by the PTU. The marker MUST be added 
	 * if it does not already exist, otherwise it MUST be updated. The ControlMessage.payload 
	 * MUST be a Marker.Marker.
	 */
	UPDATE_MARKER,
	/* Request that a marker be removed by the PTU. The ControlMessage.payload MUST be 
	 * the UUID.UUID of the marker to be removed and the PTU MUST remove it.
	 */
	DELETE_MARKER,
	/* Request that PTU clear all markers. ControlMessage.payload MUST be 
	 * null.
	 */
	CLEAR_ALL_MARKERS,
	/* Request that a map overlay be updated by the PTU. The overlay MUST be added 
	 * if it does not already exist, otherwise it MUST be updated. The ControlMessage.payload 
	 * MUST be a Navigation.MapOverlay.
	 */
	UPDATE_MAP_OVERLAY,
	/* Request that a map overlay be removed by the PTU. The ControlMessage.payload MUST be 
	 * the UUID.UUID of the map overlay to be removed and the PTU MUST remove it.
	 */
	DELETE_MAP_OVERLAY,
	/* Request that PTU clear all map overlays. ControlMessage.payload MUST be 
	 * null.
	 */
	CLEAR_ALL_MAP_OVERLAYS
}

struct StartRecordingControlPayload {
    /** The campaign name that the measurement is part of - can be blank */
    1:string campaign, 
    /** A custom tag that can be applied to the measurement - can be blank */
    2:string tag, 
    /** A number that is used to keep track of the measurement - can be blank */
    3:i32 measurementNumber, 
    /** A short description of what the measurement is - can be blank */
    4:string description, 
    /** A short description of where the meausurement was taken - can be blank */
    5:string location, 
    /** Time in milliseconds for the PTU to run the measurement.  A zero is interpreted as infinite duration */
    6:i64 duration, 
    /** A type used to classify what the recording is */
    7:PTUPayload.RecordingType recordingType
}

/**	Members of the ControlPayloadUnion. The contract describing which 
 * 	payload must accompany which ControlType is defined in the ControlType enum above.
 */
union ControlPayloadUnion {
	1: StartRecordingControlPayload startRecordingPayload;
	2: Navigation.Waypoint waypoint;
	3: Navigation.BoundingBox bBox;
	4: WINDMessage.Message message;
	5: Marker.Marker marker;
	6: UUID.UUID id;
	7: Navigation.MapOverlay overlay;
}

/** Each ControlMessage is defined by a UUID, a required type and 
 *  a payload that may be null.
 *  The type of payload that must accompany each ControlType is defined above.
 */
struct ControlMessage {
	1: UUID.UUID commandId;
	2: ControlType type;
	3: optional ControlPayloadUnion payload;
}



/**	A list of ControlMessageAcks are sent to the CVRS by the PTU using the pushAcknowledgements method defined 
 *	below. Each one is used to acknowledge whether or not a particular ControlMessage was carried out successfully. If the 
 *	the ControlMessage could not be carried out, an exception MUST be included.
 */
struct ControlMessageAck {
 	/** This UUID of the corresponding ControlMessage */
	1: UUID.UUID commandId;
 	/** Whether or not the command was executed successfully */
	2: bool success;
	/** If it was not carried out successfully, the exception that occurred */
	3: optional Exceptions.ExceptionUnion specificException;
}

/**	Used to optionally send a RecordingConfiguration as one of the parameters of the define method
 * 	described below. 
 */
struct RecordingUpdate {
	1: optional PTUPayload.RecordingConfiguration recordingConfig;
}

/**	Used to optionally send a SystemDefinition and a SystemConfiguration as parameters of the reportStatus 
 * 	and pushData methods described below. The definition and configuration should be included only if they 
 * 	have changed in their respective contexts (i.e., the current PTU status being reported via reportStatus or 
 * 	the stream of data, which may be historical, being pushed)
 */
struct DefinitionAndConfigurationUpdate {
	1: optional PTUPayload.SystemDefinition systemDefinition;
	2: optional PTUPayload.SystemConfiguration systemConfiguration;
}

/** CVRSEndpoint

This is the service implemented by WIND-compliant CVRS
software, and provides a mechanism for the PTU to push
data, status, and other information to the CVRS.

This service defines a basic flow for connecting to a
CVRS and sending it data. Each time a PTU re-establishes
connection to the CVRS, the PTU MUST call these methods
in the following order:

	1. registerPtu
	2. define
	loop (1hz):
		3. reportStatus
		4. pushData
		5. pushAcknowledgements

The initial registration establishes a session between the PTU
and the CVRS which can be used to track the state of the connection.
The CVRS MAY determine that a session has become invalid if it has
not received a message from the PTU within a reasonable period of time.

The specific semantics of each method are described in the
comments on the method definitions below.
**/
service CVRSEndpoint
{
	 /* registerPtu - Initiate a session

	 This method establishes a unique ID for this connection, to
	 allow the CVRS to track subsequent service calls and ensure
	 that the protocol is being followed. This also allows the CVRS
	 to identify and report an incompatible protocol version if
	 necessary.

	 If the protocol is compatible, the CVRS MUST send back a Session object with
	 the StatusCode set to OK and a randomly generated UUID for the sessionId. If
	 in any subsequent call, the sessionId sent by the PTU  does not match the one
	 returned from the registerPtu() method, the CVRS MUST throw an InvalidSession
	 error. Upon receipt of such an error, the PTU MUST begin the flow again by
	 re-registering with the CVRS.

	 If the protocol is incompatible, the CVRS MUST send back a Session object
	 with the StatusCode set to PROTOCOL_MISMATCH and a null UUID, at which
	 point the PTU MUST disconnect from the CVRS.

	 Args:
	 		unitDefinition: A valid definition for this PTU.

	 Returns:
	 		Session: a session object either reporting an error or a sessionId.

	 */
	 Session registerPtu(1: PTUPayload.UnitDefinition unitDefinition);

	 /* define - define the PTU to the CVRS once a session is established.

	 The PTU MUST send the current Status, SystemDefinition, and SystemConfiguration
	 to the CVRS. If a recording is currently active, then the PTU MUST send
	 the RecordingConfiguration associated with that recording as well.

	 Args:
	 		sessionId: This MUST match the one returned from the registerPtu()
					 method. If the sessionId does not match, the CVRS MUST throw an InvalidSession
					 error; upon receipt of such an error, the PTU MUST begin the flow again by
					 re-registering with the CVRS.

			status: The current Status of the PTU.

			systemDefinition: The current SystemDefinition

			systemConfiguration: The current SystemConfiguration

			recordingUpdate: A RecordingUpdate that contains a RecordingConfiguration ONLY if a recording is active

	 Returns:
	 		bool: true if the data was successfully received; false otherwise.
						If the value is false, the PTU MUST re-send this data payload again.

	 */
	 bool define(
		 1: UUID.UUID sessionId,
		 2: PTUPayload.Status status,
		 3: PTUPayload.SystemDefinition systemDefinition,
		 4: PTUPayload.SystemConfiguration systemConfiguration,
		 5: RecordingUpdate recordingUpdate
	 ) throws (1: Exceptions.InvalidSession error);

	 /* reportStatus - report PTU status

	 This method provides a mechanism for the PTU to report health status and
	 configuration to the CVRS. The PTU SHOULD call this method once every second.

	 Args:
	 		sessionId: This MUST match the one returned from the registerPtu()
					 method. If the sessionId does not match, the CVRS MUST throw an InvalidSession
					 error; upon receipt of such an error, the PTU MUST begin the flow again by
					 re-registering with the CVRS.

			status: The current Status of the PTU.

			definitionAndConfigurationUpdate: A DefinitionAndConfigurationUpdate that contains SystemDefinition 
											  and SystemConfiguration ONLY if they have changed

	 Returns:
	 		List<ControlMessage>: Defines any actions to be taken by the PTU. See the definition
					 of ControlMessage for the possible actions.
	 */
	 list<ControlMessage> reportStatus(
		 1: UUID.UUID sessionId,
		 2: PTUPayload.Status status,
		 3: DefinitionAndConfigurationUpdate definitionAndConfigurationUpdate

	 ) throws (1: Exceptions.InvalidSession error);

	 /* pushData - Push data to the CVRS

	 This method provides a mechanism for the PTU to push data to the CVRS.  See PTUPayload.thrift->DataPayload for
	 an explanation for how to divide the data for the datum.

	 Args:
	 		sessionId: This MUST match the one returned from the registerPtu()
					 method. If the sessionId does not match, the CVRS MUST throw an InvalidSession
					 error; upon receipt of such an error, the PTU MUST begin the flow again by
					 re-registering with the CVRS.

			datum: The latest DataPayload to be sent to the CVRS.

			definitionAndConfigurationUpdate: A DefinitionAndConfigurationUpdate that contains SystemDefinition 
											  and SystemConfiguration corresponding to the datum being returned 
											  ONLY if they have changed with respect to the last pushData call.


	 Returns:
	 		bool: true if the data was successfully received; false otherwise.
						If the value is false, the PTU MUST re-send this data payload again.
	 */
	 bool pushData(
		 1: UUID.UUID sessionId,
		 2: PTUPayload.DataPayload datum,
		 3: DefinitionAndConfigurationUpdate definitionAndConfigurationUpdate
	 ) throws (1: Exceptions.InvalidSession error);

	 /* pushAcknowledgements - Push acknowledgements of control messages to the CVRS

	 This method provides a mechanism for the PTU to acknowledge whether or not each ControlMessage command
	 was executed successfully.

	 Args:
	 		sessionId: This MUST match the one returned from the registerPtu()
					 method. If the sessionId does not match, the CVRS MUST throw an InvalidSession
					 error; upon receipt of such an error, the PTU MUST begin the flow again by
					 re-registering with the CVRS.

			aknowledgements: A list of ControlMessageAcks correspong to ControlMessages received by the PTU.

	 Returns:
	 		bool: true if the acnowledgements were successfully received; false otherwise.
				  If the value is false, the PTU MUST re-send this list of acknowledgements again.
	 */
	 bool pushAcknowledgements(
		 1: UUID.UUID sessionId,
		 2: list<ControlMessageAck> acknowledgements
	 ) throws (1: Exceptions.InvalidSession error);
}
