
from wind_daq.thrift.pyout.PTUServices import Client
from wind_daq.thrift.pyout.PTUPayload import (RecordingConfiguration,
                                              DataPayload,
                                              Status)
from wind_daq.thrift.pyout.Health import Health
from .configuration import (SYSTEM_CONFIGURATION,
                            SYSTEM_DEFINITION)
import thrift_uuid
import time
import numpy as np
import os


class CAEN_Digitizer(Client):

    def __init__(self):
        # Create method to initialize the detectors present on the CAEN machine.
        # This will populate self.system_configuration, or some sort of dictionary.
        self.isRecording = False
        self.isOnline = True
        self.name = "CAEN Digitizer"
        # HACK: Faking constant hard drive and battery stuff
        self.hardDriveUsedPercent = 25.0
        self.batteryRemainingPercent = 25.0

        # HACK: Health is fixed
        self.health = Health('Nominal')


        #  Describe the system itself. Currently we have 2 hookups, both NaI cylinders.
        self.systemConfiguration = SYSTEM_CONFIGURATION
        self.recordingContainer = {}

        # intialize system status
        self.status = Status(unitId=self.systemConfiguration.unitId,
                             isRecording=self.isRecording,
                             recordingId=None,
                             hardDriveUsedPercent=self.hardDriveUsedPercent,
                             batteryRemainingPercent=self.batteryRemainingPercent,
                             systemTime=self._get_posix_time())

        self.SystemDefinition = SYSTEM_DEFINITION

        # Data stuff
        self.num_bins = 2**15

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
        self.isOnline = False
        self.recordingContainer = {}
        self.isOnline = True

    def exit(self):
        """
        Exits the data acquisition software
        """
        print('Exiting DAQ software.')
        self.isOnline = False
        self.recordingContainer = {}

    def shutdown(self):
        """
        Powers down the PTU
        """
        print('Shutting down PTU')
        self.isOnline = False
        self.recordingContainer = {}

    def startRecording(self, campaign, tag, measurementNumber, description,
                       location, duration, recordingType):
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

        # I have to instantiate the missing information
        #    to build out the missing information

        recordingId = thrift_uuid.generate_thrift_uuid()
        unitId = self.systemConfiguration.unitId
        filename = self._generate_filename(recordingId,
                                           campaign,
                                           tag,
                                           measurementNumber,
                                           recordingType,
                                           duration)
        POSIXStartTime = self._get_posix_time()
        self.recordingId = recordingId
        self.recordingConfiguration = RecordingConfiguration(
            unitId=unitId,
            recordingId=recordingId,
            campaign=campaign,
            tag=tag,
            description=description,
            location=location,
            fileName=filename,
            recordingType=recordingType,
            recordingDuration=duration,
            POSIXStartTime=POSIXStartTime,
            measurementNumber=measurementNumber,)
        self.isRecording = True
        return self.recordingConfiguration

    def getRecordingConfiguration(self, recordingId):
        """
        Retrieves the recording configuration of a recording in progress or the last recording performed
        if currently not in a recording or null if a recording has not yet been performed this session

        Parameters:
         - recordingId
        """
        return self.recordingConfiguration

    def setRecordingDuration(self, duration):
        """
        Updates the recording duration.  Duration is changed only if recording is active. If duration
        is less than (systemTime - POSIXStartTime) recording is stopped and duration is set
        to (systemTime - POSIXStartTime)

        Parameters:
         - duration
        """
        systemTime = self._get_posix_time()
        POSIXStartTime = self.recordingConfiguration.POSIXStartTime
        if duration < (systemTime - POSIXStartTime):
            self.endRecording()
            self.recordingConfiguration.recordingDuration = (systemTime - POSIXStartTime)
        else:
            self.recordingConfiguration.recordingDuration = duration
        return

    def getRecordings(self):
        """
        Returns all recordings currently on the PTU.
        """
        # TODO: What is the definition of recordings? As in all the filenames?
        #       All the runs done on the PTU for this session?
        #       Everything in this campaign?????
        return self.recordingConfiguration  # this is probably not correct

    def endRecording(self):
        """
        Ends a measurement recording.  If called before recording has reached duration
        recording is stopped and duration is updated.

        Returns True if recording stopped successfully, False if otherwise
        """
        try:
            self.setRecordingDuration(self.recordingConfiguration.recordingDuration)
            self.recordingContainer[self.recordingId] = self.getRecording
            self.isRecording = False
            return True
        except:
            return False

    def getStatus(self):
        """
        Gets basic information from the PTU, such as name and current state.
        This is not meant to be called more than 1 time per second.
        """
        # I have to update status whenever this is called
        self.status.isRecording = self.isRecording
        self.status.recordingId = self.recordingId
        # HACK: Set to a constant value
        self.status.hardDriveUsedPercent = 25.0
        self.status.batteryRemainingPercent = 25.0
        self.status.systemTime = self._get_posix_time()
        return self.status

    def getUnitDefinition(self):
        """
        Gets basic information about the Unit, (name, versions, etc...)
        """
        # Access self.system_configuration, return the info in herent to that.
        # consider using an ordered dictionary to get all the keys called in the correct order,
        #     then construct the string out
        return self.UnitDefinition

    def getSystemDefinition(self):
        """
        Gets the definition of the system, defining what the system is capable of
        """
        return self.SystemDefinition

    def getSystemConfiguration(self):
        """
        Gets how the system is configured, such as high voltages, gain, etc...

        Returns current system configuration
        """
        return self.SystemConfiguration

    def setSystemConfiguration(self, systemConfig):
        """
        Sets system configuration options.

        Returns updated system configuration

        Parameters:
         - systemConfig
        """
        self.systemConfiguration = systemConfig
        return self.systemConfiguration

    def getLatestData(self, requestedData):
        """
        MAY be used both during recording and when not recording and is used as a way to always get the latest data from
        the system regardless of recording state.  This call MUST return the latest complete data packet (within the
        given filter).  This is used to keep the CVRS and PTU UI updated when not actively recording.

        MUST return the latest data packet regardless if in an active recording or not.

        Parameters:
         - requestedData
        """
        # Requested Data = detector index
        gammaSpectrumData = []
        gammaListData = []
        gammaGrossCountData = []
        gammaDoseData = []  # TODO: the fuck is this?

        for i in range(len(requestedData)):
            desired = requestedData[i]  # this represents the desired detector
            gammaSpectrumData += [self._get_spectra(desired, -1)]
            gammaListData += [self._get_list(desired, -1)]
            gammaGrossCountData += [self._get_counts(desired, -1)]
            gammaDoseData += [self._get_dose(desired, -1)]

        data = DataPayload(unitId=self.systemConfiguration.componentId,
                           timeStamp=None,  # TODO: Fill this
                           systemHealth=self.health,
                           isEOF=not self.isRecording,
                           recordingConfig=self.recordingConfiguration,
                           # here all the way after are list
                           gammaSpectrumData=gammaSpectrumData,
                           gammaListData=gammaListData,
                           gammaGrossCountData=gammaGrossCountData,
                           gammaDoseData=gammaDoseData)
        return data

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
        # Requested Data = detector index

        if recordingId in self.recordingContainer:
            gammaSpectrumData = []
            gammaListData = []
            gammaGrossCountData = []
            gammaDoseData = []  # TODO: the fuck is this?

            # generate the time index
            timeindex = self._get_timeindex(self, lastTime)

            for i in range(len(requestedData)):
                desired = requestedData[i]  # this represents the desired detector
                gammaSpectrumData += [self._get_spectra(desired, timeindex)]
                gammaListData += [self._get_list(desired, timeindex)]
                gammaGrossCountData += [self._get_counts(desired, timeindex)]
                gammaDoseData += [self._get_dose(desired, timeindexb)]

            data = DataPayload(unitId=self.systemConfiguration.componentId,
                               timeStamp=None,  # TODO: Fill this
                               systemHealth=self.health,
                               isEOF=not self.isRecording,
                               recordingConfig=self.recordingConfiguration,
                               # here all the way after are list
                               gammaSpectrumData=gammaSpectrumData,
                               gammaListData=gammaListData,
                               gammaGrossCountData=gammaGrossCountData,
                               gammaDoseData=gammaDoseData)
            return data
        else:
            return Exceptions.RetrievalError
        return

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

        # startTime = POSIX Time * 1000
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

    def _generate_filename(self, recordingId, campaign, tag, measurementNumber,
                           recordingType, duration):
        filename = '{}_{}_{}_{}_{}_{}'.format(recordingId,
                                              campaign,
                                              tag,
                                              measurementNumber,
                                              recordingType,
                                              duration)
        return filename

    def _get_timeindex(self, timesince):
        # TODO: Make sure that POSIXStartTime and tagtime are of the same units
        POSIXStartTime = self.recordingConfiguration.POSIXStartTime  # do i need this??
        tagtime_list = self._get_tagtime()
        tagtime_length = len(tagtime_list)
        if timesince < POSIXStartTime:
            # you want the entire dataset from the recording.
            # Which means, you don't need to do an index search.
            # create an index list which spans all of tagtime
            timeindex = np.arange(tagtime_length)
            return timeindex
        for i in range(tagtime_length):
            # have to approach from the opposite side. We're going to step backwards in time.
            index = tagtime_length-1-i  # -1 because indexing starts at 0.
            if tagtime_list[index] < timesince:
                # I've found the time, I just need to add one since I've gone slightly too far.
                index += 1
                break
        base_index = np.arange(tagtime_length)
        timeindex = np.add(base_index, np.repeat(index, (len(base_index),)))
        return timeindex

    def _get_posix_time(self):
        return int(time.time() * 1000.0)

    def _get_spectra(self, det_name):
        pass

    def _get_counts(self, det_name):
        pass

    def _get_list(self, det_name):
        pass

    def _get_dose(self, det_name):
        pass

    def _get_tagtime(self):
        return

    def follow(self, thefile):
        # From https://stackoverflow.com/questions/3290292/read-from-a-log-file-as-its-being-written-using-python
        thefile.seek(0, 2)  # Go to the end of the file
        #  yield None # Sleep briefly
        time.sleep(0.01)
        # Catching time.
        line = thefile.readline()
        # continue
        yield line

    def _set_daq_files(self, file_list):
        self.filehandles = []
        self.daq_generators = []
        for x in file_list:
            filehandle = open(x, 'r')
            self.filehandles += [filehandle]
            self.daq_generators += [self.follow(filehandle)]
        return

    def _unset_daq_files(self):
        for handle in self._filehandles:
            handle.close()
        self._filehandles = []
        self.daq_generators = []
        return

    def _collect_daq_data(self):
        # I don't think I can measure and return data simultaneously without threading
        #       and messing with semaphores or race conditions.

        # Artificially enforce an integration time. In the while, loop into a status checker, kind of like firmware in an MCU.

        # Enforce that we only collect 100 ms worth of data at a time.

        collectionStartTime = self._get_posix_time()

        triggerTimeTag = []
        energyDeposited = []

        # calibration factors, assuming linear calibration y=mx+b for now
        m = 0.2
        b = 0
        # Amount of time in seconds want to wait before checking pipe for more data
        # bufferTime = 0.1  # 100 ms
        i = 0  # channel to read.
        # while is for round robin implementation.
        while self.isRecording and (self._get_posix_time()-collectionStartTime < 100):
            generator = self.daq_generators[i]
            for line in generator:
                if line:
                    line = line.strip()
                    words = line.split()  # split line into space delimited words
                    #trigger time is in clock clicks 4ns/tick, seems to start as some insanely large number so subtracting out as initial time

                    #build 1D arrays with list mode data
                    # also putting trigger time in seconds instead of clock ticks and calibrating
                    # tagtime = float(words[0])  # to seconds

                    # I don't think I care about trigger time.
                    time = float(words[0]) / 4E9 * 1000.0
                    triggerTimeTag.append(time-self.POSIXStartTime)  # trigger time in ns (4ns/clock tick)
                    energyDeposited.append((float(words[1])*m) + b)  # total integrated energy of event in rough keV

                    #uncomment for debugging
                    #print("Qlong is: " + str(qlong))
            self.daq_generators[i] = self.follow(self.filehandles[i])
            i += 1
            if i == len(self.daq_generators):
                # round robin implementation
                i = 0

        # Which means the recording stopped for reading or for some other reason.
        listmode_triggerTimeTag = np.array(triggerTimeTag)
        listmode_energyDeposited = np.array(energyDeposited)
        #build a histogram that can be called later
        histEnergyDeposited, bin_edges = np.histogram(energyDeposited, bins=range(self.num_bins))
        histEnergyDeposited = histEnergyDeposited

        ###### OVER KILL BOIS ######
        # make max time want to plot in 100's of ms for proper binning
        # maxTime = int(math.ceil(triggerTimeTag[len(triggerTimeTag)-1])/100000)
        # print(maxTime) #uncomment for debugging

        # This is unnecessary for now. This is for poking around in higher fidelity.
        # build 2D histogram with energy, time, counts (x,y,z)
        # make 2D histogram, bin (with xedges and yedges) to 100ms/division for time
        # histTime, xedges, yedges = np.histogram2d(qlong,triggerTimeTag, bins = [num_bins, maxTime])
        ###### OVER KILL BOIS ######
        return listmode_triggerTimeTag, listmode_energyDeposited, histEnergyDeposited












