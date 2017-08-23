from wind_daq.thrift.pyout.PTUS import Client
import numpy as np

# Enumerate types are just basic dictionaries
#      to index location


RECORDINGTYPE2ENUM = {
    'Other':       1,
    'Calibration': 2,
    'Measurement': 3,
    'Background':  4,
    'Search':      5
}

UNITTYPE2ENUM = {
    'Wearable':    1,
    'Luggable':    2,
    'Portal':      3,
    'Stationary':  4,
    'Aerial':      5,
    'Vehicle':     6,
    'Source':      7,
    'Other':       8
}


class CAEN_Digitizer(Client):

    def __init__(self):
        # Create method to initialize the detectors present on the CAEN machine.
        # This will populate self.system_configuration, or some sort of dictionary.

    def ping(self):
        """
        Simple test for determining Thrift connectivity.

        Returns The string literal "pong"
        """
        return "pong"

    def restart(self):
        """
        Tells the system to reboot
        """
        print('System restarting...')
        self.isOnline = True

    def exit(self):
        """
        Exits the data acquisition software
        """
        print('Exiting DAQ software.')
        self.isOnline = False

    def shutdown(self):
        """
        Powers down the PTU
        """
        print('Shutting down PTU')
        self.isOnline = False

    def startRecording(self, campaign, tag, measurementNumber, description, location, duration, recordingType):
        """
        Starts a measurement recording.

        Returns an object filled in with recording information

        Parameters:
         - campaign: The campaign name that the measurement is part of - can be blank
         - tag: A custom tag that can be applied to the measurement - can be blank
         - measurementNumber: A number that is used to keep track of the measurement - can be blank
         - description: A short description of what the measurement is - can be blank
         - location: A short description of where the meausurement was taken - can be blank
         - duration: Time in milliseconds for the PTU to run the measurement.  A zero is interpreted as infinite duration
         - recordingType: A type used to classify what the recording is
        """

        # I have to instanteiate the missing information
        #    to build out the missing information

        recording_object = RecordingConfiguration(
            unitId=None,
            recordingId=None,
            campaign=campaign,
            tag=tag,
            description=description,
            location=location,
            fileName=None,
            recordingType=recordingType,
            recordingDuration=duration,
            POSIXStartTime=None,
            measurementNumber=measurementNumber,):

        return recording_object

    def getRecordingConfiguration(self, recordingId):
        """
        Retrieves the recording configuration of a recording in progress or the last recording performed
        if currently not in a recording or null if a recording has not yet been performed this session

        Parameters:
         - recordingId
        """

        # is this really a dictionary
        return self.recording_container[recordingId]

    def setRecordingDuration(self, duration):
        """
        Updates the recording duration.  Duration is changed only if recording is active. If duration
        is less than (systemTime - POSIXStartTime) recording is stopped and duration is set
        to (systemTime - POSIXStartTime)

        Parameters:
         - duration
        """
        self.recording_container[]

    def getRecordings(self):
        """
        Returns all recordings currently on the PTU.
        """
        pass

    def endRecording(self):
        """
        Ends a measurement recording.  If called before recording has reached duration
        recording is stopped and duration is updated.

        Returns True if recording stopped successfully, False if otherwise
        """
        pass

    def getStatus(self):
        """
        Gets basic information from the PTU, such as name and current state.
        This is not meant to be called more than 1 time per second.
        """
        return self.isOnline

    def getUnitDefinition(self):
        """
        Gets basic information about the Unit, (name, versions, etc...)
        """
        # Access self.system_configuration, return the info in herent to that.
        # consider using an ordered dictionary to get all the keys called in the correct order,
        #     then construct the string out
        pass

    def getSystemDefinition(self):
        """
        Gets the definition of the system, defining what the system is capable of
        """
        # Define some string
        pass

    def getSystemConfiguration(self):
        """
        Gets how the system is configured, such as high voltages, gain, etc...

        Returns current system configuration
        """
        # System config??
        pass

    def setSystemConfiguration(self, systemConfig):
        """
        Sets system configuration options.

        Returns updated system configuration

        Parameters:
         - systemConfig
        """
        # systemConfig is a dictionary???
        pass

    def getLatestData(self, requestedData):
        """
        MAY be used both during recording and when not recording and is used as a way to always get the latest data from
        the system regardless of recording state.  This call MUST return the latest complete data packet (within the
        given filter).  This is used to keep the CVRS and PTU UI updated when not actively recording.

        MUST return the latest data packet regardless if in an active recording or not.

        Parameters:
         - requestedData
        """
        # What is requested data? recordingIDnumber??? What's the bahaviour for initializing?
        pass

    def getDataSinceTime(self, recordingId, lastTime, requestedData):
        """
         * Retreives all data since lastTime.
         *
         * The recordingId SHOULD match a recordingId stored by the system (either a current active recording
         * or a previously saved recording). If the recordingId does not exist on the system, the system MUST
         * return an Exceptions.RetrievalError error.

         * If the recordingId does exist on the system but no DataPayloads exist within the specified time window,
         * the system MUST return an empty list.

        Parameters:
         - recordingId
         - lastTime
         - requestedData
        """
        # Requested data is the data container????
        pass

    def getDataSinceTimeWithLimit(self, recordingId, lastTime, limit, requestedData):
        """
        Retrieves all data since lastTime up to a given limit and must sort DataPayloads by time, ascending,
        before cutting off the data..  This is useful for "paging" data when a lot of data needs
        to be retrieved.

        The recordingId SHOULD match a recordingId stored by the system (either a current active recording
        or a previously saved recording). If the recordingId does not exist on the system, the system MUST
        return an Exceptions.RetrievalError error.

        If the recordingId does exist on the system but no DataPayloads exist within the specified time window,
        the system MUST return an empty list.

        Parameters:
         - recordingId
         - lastTime
         - limit
         - requestedData
        """
        # Format of the lastTime????
        pass

    def getDataInTimeWindow(self, recordingId, startTime, endTime, requestedData):
        """
        * Retrieve all DataPayloads for a recording identified by recordingId in the time window specified as being
        # greater than or equal to startTime and less than endTime.
        *
        * The recordingId SHOULD match a recordingId stored by the system (either a current active recording
        * or a previously saved recording). If the recordingId does not exist on the system, the system MUST
        * return an Exceptions.RetrievalError error.

        * If the recordingId does exist on the system but no DataPayloads exist within the specified time window,
        * the system MUST return an empty list.

        Parameters:
         - recordingId
         - startTime
         - endTime
         - requestedData
        """
        pass

    def getDataInTimeWindowWithLimit(self, recordingId, startTime, endTime, limit, requestedData):
        """
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

        Parameters:
         - recordingId
         - startTime
         - endTime
         - limit
         - requestedData
        """
        pass

    def getDataFile(self, fileName):
        """
        Retrieves a specific file off the PTU. Used for synchronizing data between the PTU and CVRS.  Also can
        be used to retrieve context sensor information after or during a recording.  Consider replacing this with
        an id?

        Parameters:
         - fileName: The name of the file to pull - PTU is responsible for knowing where this file is located
        """
        pass

    def sendCommand(self, componentId, command):
        """
        Used to send commands to specific sensors/algorithms. Examples of this include resetting background for
        an algorithm or changing modes (from wide to narrow angle) on a camera.  This is not meant to be used
        to change configuration items such as high voltage and gain, which is accomplished via setSystemConfiguration

        Parameters:
         - componentId
         - command
        """
        pass

    def addWaypoint(self, name, latitude, longitude, altitude):
        """
        Adds a waypoint to the system.

        Returns new waypoint added

        Parameters:
         - name
         - latitude
         - longitude
         - altitude
        """
        pass

    def editWaypoint(self, waypoint):
        """
        Edits an existing waypoint in the system.

        Returns Altered waypoint

        Parameters:
         - waypoint
        """
        pass

    def deleteWaypoint(self, waypointId):
        """
        Removes a waypoint in the system

        Parameters:
         - waypointId
        """
        pass

    def getWaypoints(self):
        """
        Returns a list of the active waypoints in the system.

        Returns list of active waypoints
        """
        pass

    def addBoundingBox(self, name, vertices, type):
        """
        Creates a navigational bounding box for directing searches or calling out areas of interest.

        Returns new bounding box object

        Parameters:
         - name
         - vertices
         - type
        """
        pass

    def editBoundingBox(self, boundingBox):
        """
        Edits an existing navigational bounding box.

        Returns new bounding box object

        Parameters:
         - boundingBox
        """
        pass

    def deleteBoundingBox(self, boundingBoxId):
        """
        Removes a bounding box in the system

        Parameters:
         - boundingBoxId
        """
        pass

    def getBoundingBoxes(self):
        """
        Returns a list of the current bounding boxes in the system.

        Returns list of active bounding boxes
        """
        pass

    def addMessage(self, message, timeStampSent):
        """
        Creates a new message.

        @returns New message object

        Parameters:
         - message
         - timeStampSent
        """
        pass

    def editMessage(self, message):
        """
        Edits an existing message.

        Returns the new message object

        Parameters:
         - message
        """
        pass

    def deleteMessage(self, messageId):
        """
        Removes a message in the system

        Parameters:
         - messageId
        """
        pass

    def getMessages(self):
        """
        Returns a list of the current messages in the system.

        Returns List of active messages
        """
        pass

    def addMarker(self, name, type, latitude, longitude, altitude):
        """
        Creates a new marker.

        @returns New marker object

        Parameters:
         - name
         - type
         - latitude
         - longitude
         - altitude
        """
        pass

    def editMarker(self, marker):
        """
        Request an edit to an existing Marker by providing a new copy of it.

        The Marker object provided SHOULD have a markerId that matches an existing
        Marker stored by the system. If the markerId does not exist, the system MUST return
        an Exceptions.UpdateError error.

        The system MUST replace the data in the existing Marker for the following fields:
          + timeStamp
          + name
          + MarkerType
          + latitude
          + longitude
          + altitude
          + gammaData
          + neutronData
          + environmentalData
          + algorithmData

        The system MUST NOT change the data in the existing Marker for the following fields,
        unless the previous data was NULL. If the Marker passed in differs from the existing
        Marker, the system MUST throw an UpdateError with an appropriate message:
          + unitId

        If the marker is updated successfully, this method MUST return the stored Marker object
        after the transformations above were completed.

        Parameters:
         - marker
        """
        pass

    def deleteMarker(self, markerId):
        """
        Removes a marker in the system

        Parameters:
         - markerId
        """
        pass

    def getMarkers(self):
        """
        Returns a list of the current markers in the system.

        Returns List of active markers
        """
        pass

    def registerAlgorithm(self, ipAddress, port, definition):
        """
        Registers an algorithm with the PTU.  After registration, the PTU will send the Algorithm the PTU
        definition via the AlgorithmServices function definePTU and then begin sending data at the integration rate
        via the AlgorithmServices function updateAlgorithm.

        MUST send the following parameters:
          ipAddress:  The IP address that the PTU Thrift client will be able to connect to the algorithm on
          port:       The port that that PTU Thrift client will be able to connect to the algorithm on
          definition: Defines to the PTU what the algorithm is capable of - defined in AlgorithmPayload

        Returns to the Algorithm if the Algorithm has been registered properly and will imminently receive the
        PTU definition.

        Parameters:
         - ipAddress
         - port
         - definition
        """
        pass
