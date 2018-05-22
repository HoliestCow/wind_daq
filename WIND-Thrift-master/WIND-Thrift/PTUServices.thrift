namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "UUID.thrift"
include "PTUPayload.thrift"
include "Exceptions.thrift"
include "Navigation.thrift"
include "WINDMessage.thrift"
include "Command.thrift"
include "Marker.thrift"
include "AlgorithmPayload.thrift"


service PTU
{
    // System level commands
    
    /**
     * Simple test for determining Thrift connectivity.
     * 
     * Returns The string literal "pong"
     */
    string ping();

    /**
     * Tells the system to reboot
     */
    void restart(); 

    /**
     * Exits the data acquisition software
     */
    void exit();

    /**
     * Powers down the PTU
     */
    void shutdown();

    // Recording commands

    /**
     * Starts a measurement recording.
     *
     * Returns an object filled in with recording information
     */
    PTUPayload.RecordingConfiguration startRecording(
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
        7:PTUPayload.RecordingType recordingType) throws (1:Exceptions.RecordingError error);

    /**
     * Retrieves the recording configuration of a recording in progress or the last recording performed
     * if currently not in a recording or null if a recording has not yet been performed this session
     */
    PTUPayload.RecordingConfiguration getRecordingConfiguration(1:UUID.UUID recordingId);

    /**
     * Updates the recording duration.  Duration is changed only if recording is active. If duration 
     * is less than (systemTime - POSIXStartTime) recording is stopped and duration is set
     * to (systemTime - POSIXStartTime)
     */
    PTUPayload.RecordingConfiguration setRecordingDuration(1:i64 duration)

    /**
     * Returns all recordings currently on the PTU.
     */
    list<PTUPayload.RecordingConfiguration> getRecordings();

    /**
     * Ends a measurement recording.  If called before recording has reached duration
     * recording is stopped and duration is updated.
     *
     * Returns True if recording stopped successfully, False if otherwise
     */
    bool endRecording() throws (1:Exceptions.RecordingError error);

    /**
     * Deletes the one or more recordings specified by the list of recording UUIDs.
     *
     * Returns true if recordings were deleted successfully and false otherwise.
     */
     bool deleteRecordings(1:list<UUID.UUID> recordingIds) throws (1:Exceptions.RecordingError error);

    // Status

    /** 
     * Gets basic information from the PTU, such as name and current state.
     * This is not meant to be called more than 1 time per second.
     */
    PTUPayload.Status getStatus();

    // Defintions

    /**
     * Gets basic information about the Unit, (name, versions, etc...)
     */
    PTUPayload.UnitDefinition getUnitDefinition();

    /**
     * Gets the definition of the system, defining what the system is capable of
     */
    PTUPayload.SystemDefinition getSystemDefinition();

    // Configuration

    /**
     * Gets how the system is configured, such as high voltages, gain, etc...
     *
     * Returns current system configuration
     */
    PTUPayload.SystemConfiguration getSystemConfiguration();

    /**
     * Sets system configuration options.
     *
     * Returns updated system configuration
     */
    PTUPayload.SystemConfiguration setSystemConfiguration(1:PTUPayload.SystemConfiguration systemConfig) throws (1:Exceptions.UpdateError error);

    // Data

    /**
     * MAY be used both during recording and when not recording and is used as a way to always get the latest data from
     * the system regardless of recording state.  This call MUST return the latest complete data packet (within the
     * given filter).  This is used to keep the CVRS and PTU UI updated when not actively recording.
     *
     * MUST return the latest data packet regardless if in an active recording or not.
     *
     * See PTUPayload.thrift->DataPayload for an explanation for how to divide the data for the datum.
     */
    PTUPayload.DataPayload getLatestData(1:PTUPayload.DataFilter requestedData);

    /**
     * Retreives all data since lastTime.
     * 
     * The recordingId SHOULD match a recordingId stored by the system (either a current active recording
     * or a previously saved recording). If the recordingId does not exist on the system, the system MUST 
     * return an Exceptions.RetrievalError error.
     *
     * If the recordingId does exist on the system but no DataPayloads exist within the specified time window, 
     * the system MUST return an empty list.
     *
     * See PTUPayload.thrift->DataPayload for an explanation for how to divide the data for the datum.
     */
    list<PTUPayload.DataPayload> getDataSinceTime(1:UUID.UUID recordingId, 2:i64 lastTime, 3:PTUPayload.DataFilter requestedData);

    /**
     * Retrieves all data since lastTime up to a given limit and must sort DataPayloads by time, ascending, 
     * before cutting off the data..  This is useful for "paging" data when a lot of data needs 
     * to be retrieved.
     * 
     * The recordingId SHOULD match a recordingId stored by the system (either a current active recording
     * or a previously saved recording). If the recordingId does not exist on the system, the system MUST 
     * return an Exceptions.RetrievalError error.

     * If the recordingId does exist on the system but no DataPayloads exist within the specified time window, 
     * the system MUST return an empty list.
     *
     * See PTUPayload.thrift->DataPayload for an explanation for how to divide the data for the datum.
     */
    list<PTUPayload.DataPayload> getDataSinceTimeWithLimit(1:UUID.UUID recordingId, 2:i64 lastTime, 3:i32 limit, 4:PTUPayload.DataFilter requestedData);

    /**
     * Retrieve all DataPayloads for a recording identified by recordingId in the time window specified as being 
     # greater than or equal to startTime and less than endTime.
     * 
     * The recordingId SHOULD match a recordingId stored by the system (either a current active recording
     * or a previously saved recording). If the recordingId does not exist on the system, the system MUST 
     * return an Exceptions.RetrievalError error.

     * If the recordingId does exist on the system but no DataPayloads exist within the specified time window, 
     * the system MUST return an empty list.
     *
     * See PTUPayload.thrift->DataPayload for an explanation for how to divide the data for the datum.
     */
    list<PTUPayload.DataPayload> getDataInTimeWindow(1:UUID.UUID recordingId, 2:i64 startTime, 3:i64 endTime, 4:PTUPayload.DataFilter requestedData);

    /**
     * Retrieve all DataPayloads for a recording identified by recordingId in the time window specified as being 
     # greater than or equal to startTime and less than endTime. The system must retrieve no more DataPayloads than 
     * the number specified by 'limit' and must sort DataPayloads by time, ascending, before cutting off the data. 
     * Use of this method should allow for pagination of data while allowing the PTU to remain stateless. 
     * 
     * The recordingId SHOULD match a recordingId stored by the system (either a current active recording
     * or a previously saved recording). If the recordingId does not exist on the system, the system MUST 
     * return an Exceptions.RetrievalError error.

     * If the recordingId does exist on the system but no DataPayloads exist within the specified time window, 
     * the system MUST return an empty list.
     *
     * See PTUPayload.thrift->DataPayload for an explanation for how to divide the data for the datum.
     */
    list<PTUPayload.DataPayload> getDataInTimeWindowWithLimit(1:UUID.UUID recordingId, 2:i64 startTime, 3:i64 endTime, 4:i32 limit, 5:PTUPayload.DataFilter requestedData);

    /**
     * Retrieves a specific file off the PTU. Used for synchronizing data between the PTU and CVRS.  Also can 
     * be used to retrieve context sensor information after or during a recording.  Consider replacing this with
     * an id?
     */
    binary getDataFile(
        /** The name of the file to pull - PTU is responsible for knowing where this file is located */
        1:string fileName);

    // Other Commands

    /**
     * Used to send commands to specific sensors/algorithms. Examples of this include resetting background for
     * an algorithm or changing modes (from wide to narrow angle) on a camera.  This is not meant to be used
     * to change configuration items such as high voltage and gain, which is accomplished via setSystemConfiguration
     */
    void sendCommand(1:UUID.UUID componentId, 2:Command.Command command);


    // Waypoints

    /**
     * Adds a waypoint to the system.
     *
     * Returns new waypoint added
     */
    Navigation.Waypoint addWaypoint(1: string name, 2:Navigation.Location location) throws (1:Exceptions.UpdateError error);

    /**
     * Edits an existing waypoint in the system.
     *
     * Returns Altered waypoint
     */
    Navigation.Waypoint editWaypoint(1: Navigation.Waypoint waypoint);

    /**
     * Removes a waypoint in the system
     */
    void deleteWaypoint(1: UUID.UUID waypointId);

    /**
     * Returns a list of the active waypoints in the system.
     *
     * Returns list of active waypoints
     */
    list<Navigation.Waypoint> getWaypoints();

    // Bounding Boxes

    /**
     * Creates a navigational bounding box for directing searches or calling out areas of interest.
     *
     * Returns new bounding box object
     */
    Navigation.BoundingBox addBoundingBox(1: string name, 2: list<Navigation.Location> vertices, 3: Navigation.BoundingBoxType type) throws (1:Exceptions.UpdateError error);

    /**
     * Edits an existing navigational bounding box.
     *
     * Returns new bounding box object
     */
    Navigation.BoundingBox editBoundingBox(1:Navigation.BoundingBox boundingBox);

    /**
     * Removes a bounding box in the system
     */
    void deleteBoundingBox(1:UUID.UUID boundingBoxId) throws (1:Exceptions.UpdateError error);

    /**
     * Returns a list of the current bounding boxes in the system.
     *
     * Returns list of active bounding boxes
     */
    list<Navigation.BoundingBox> getBoundingBoxes();

    // Messages

    /**
     * Creates a new message.
     *
     * @returns New message object
     */
    WINDMessage.Message addMessage(1:string message, 2:i64 timeStampSent) throws (1:Exceptions.UpdateError error);

    /**
     * Edits an existing message.
     *
     * Returns the new message object
     */
    WINDMessage.Message editMessage(1:WINDMessage.Message message) throws (1:Exceptions.UpdateError error);

    /**
     * Removes a message in the system
     */
    void deleteMessage(1:UUID.UUID messageId) throws (1:Exceptions.UpdateError error);

    /**
     * Returns a list of the current messages in the system.
     *
     * Returns List of active messages
     */
    list<WINDMessage.Message> getMessages();

    // Markers
    /**
     * Creates a new marker.
     *
     * @returns New marker object
     */
    Marker.Marker addMarker(1:string name, 2:Marker.MarkerType type, 3:Navigation.Location location) throws (1:Exceptions.UpdateError error);

    /**
     * Request an edit to an existing Marker by providing a new copy of it. 
     *
     * The Marker object provided SHOULD have a markerId that matches an existing
     * Marker stored by the system. If the markerId does not exist, the system MUST return
     * an Exceptions.UpdateError error.
     *
     * The system MUST replace the data in the existing Marker for the following fields:
     *   + timeStamp
     *   + name
     *   + MarkerType
     *   + latitude
     *   + longitude
     *   + altitude
     *   + gammaData
     *   + neutronData
     *   + environmentalData
     *   + algorithmData
     * 
     * The system MUST NOT change the data in the existing Marker for the following fields,
     * unless the previous data was NULL. If the Marker passed in differs from the existing
     * Marker, the system MUST throw an UpdateError with an appropriate message:
     *   + unitId
     *
     * If the marker is updated successfully, this method MUST return the stored Marker object 
     * after the transformations above were completed.
     */
    Marker.Marker editMarker(1:Marker.Marker marker) throws (1:Exceptions.UpdateError error);

    /**
     * Removes a marker in the system
     */
    void deleteMarker(1:UUID.UUID markerId) throws (1:Exceptions.UpdateError error);

    /**
     * Returns a list of the current markers in the system.
     *
     * Returns List of active markers
     */
    list<Marker.Marker> getMarkers();

    // Map Overlays

    /**
     * Creates a map overlay useful for such purposes as the PTU operator localizing himself/herself (e.g., through a provided floorplan) or 
     * understanding progress in a search mission (e.g., through a clearing map generated by all participating PTUs)
     *
     * Returns new map overlay object with the generated UUID and timestamp
     */
    Navigation.MapOverlay addMapOverlay(1: string name, 
                                        2: Navigation.Location location, //top left
                                        3: double width, //meters
                                        4: double height, // meters
                                        5: Navigation.MapOverlayType type,
                                        6: binary image) throws (1:Exceptions.UpdateError error);

    /**
     * Edits an existing map overlay.
     */
    void editMapOverlay(1:Navigation.MapOverlay overlay) throws (1:Exceptions.UpdateError error);

    /**
     * Removes a map overlay in the system
     */
    void deleteMapOverlay(1:UUID.UUID mapOverlayId) throws (1:Exceptions.UpdateError error);

    /**
     * Returns a list of the current map overlays in the system.
     */
    list<Navigation.MapOverlay> getMapOverlays();

    /**
     * Registers an algorithm with the PTU.  After registration, the PTU will send the Algorithm the PTU
     * definition via the AlgorithmServices function definePTU and then begin sending data at the integration rate
     * via the AlgorithmServices function updateAlgorithm.
     *
     * MUST send the following parameters:
     *   uri:  The address to be used for communicating with the algorithm in URI form
     *   definition: Defines to the PTU what the algorithm is capable of - defined in AlgorithmPayload
     *
     * Returns to the Algorithm if the Algorithm has been registered properly and will imminently receive the
     * PTU definition.
     */
    bool registerAlgorithm(
        /* The address for communicating with the algorithm in URI form */
        1:string uri, 
        /* What the algorithm is capable of performing */
        3: AlgorithmPayload.AlgorithmDefinition definition);

    /**
     * Sets the address that should be used to communicate with the CVRS specified as a URI. If the PTU 
     * has an active session with a CVRS that has a different URI, that session MUST be terminated.
     */
    void setCVRSAddress(1: string uri);

    /**
     * Forces the PTU to establish a new session with the CVRS via the CVRSEndpoint service. If the PTU is able 
     * to connect and register itself successfully, the session UUID MUST be returned. If the PTU is unable to 
     * establish a session for any reason, a CVRSRegistrationError must be thrown.
     */
    UUID.UUID initiateCVRSSession() throws (1:Exceptions.CVRSRegistrationError error)
}
