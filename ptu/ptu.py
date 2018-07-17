
############### PTU STUFF ###############
import sys
import glob
import io
sys.path.append('/home/cbritt2/wind_daq/WIND-Thrift/gen-py')
sys.path.append('/home/cbritt2/')  # this is to get the wind_daq. to start working

from csvseeker import CSVSeeker

# from CVRSServices.CVRSEndpoint import Client
# from server import CVRSHandler
import CVRSServices.CVRSEndpoint
# from CVRSServices.ttypes import (StatusCode, ControlType, Session, StartRecordingControlPayload,
#                                  ControlPayloadUnion, ControlMessage, ControlMessageAck,
#                                  RecordingUpdate, DefinitionAndConfigurationUpdate)
from PTUPayload.ttypes import (UnitDefinition, UnitType, Status,
                               SystemDefinition, SystemConfiguration,
                               RecordingConfiguration, RecordingType, DataPayload)
from CVRSServices.ttypes import (RecordingUpdate,
                                  DefinitionAndConfigurationUpdate)
from GammaSensor.ttypes import (SIPMSettings, PMTSettings, SIPM_PMTSettings,
                                GammaListAndSpectrumConfiguration,
                                GammaListAndSpectrumDefinition,
                                GammaGrossCountDefinition,
                                GammaGrossCountConfiguration,
                                GammaSpectrumData,
                                GammaGrossCountData)
from DetectorCharacteristics.ttypes import (EnergyResolution, DetectorMaterial,
                                            EnergyCalibration)
from Angular.ttypes import (AngleEfficiencyPair, AngularEfficiencyDefinition)
from Component.ttypes import (GridPositionAndOrientation, ComponentDefinition)
from Common.ttypes import (Vector, Quaternion)
from PhysicalDimensions.ttypes import (RectangularDimensions)
from NavigationSensor.ttypes import (NavigationOutputDefinition,
                                     NavigationSensorDefinition,
                                     FrameOfReference,
                                     GeodeticDatum)
from Navigation.ttypes import(Waypoint, Location)
from EnvironmentalSensor.ttypes import (EnvironmentalTypes,
                                        EnvironmentalSensorDefinition)
from ContextSensor.ttypes import (ContextVideoConfiguration,
                                  ContextVideoDefinition,
                                  CameraIntrinsics)
from Spectrum.ttypes import (SpectrumResult, Spectrum, DoubleSpectrum,
                             SpectrumFormat)
from Health.ttypes import (Health)

# from server import

from UUID.ttypes import UUID

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

import time
import datetime as dt

############ Database Stuff ##############

from wind_daq.utils.thrift_uuid import Thrift_UUID
from wind_daq.utils.database import DatabaseOperations
import numpy as np
import wind_daq.ptu.catch_measurements

######### VN 300 Libraries ##########
sys.path.append('/home/cbritt2/wind_daq/libs/vnproglib-1.1.4.0/python/build/lib.linux-x86_64-3.5')
from vnpy import *

##### Other utilities

from threading import Thread

########## DONE IMPORTING ################


class PTU:
    def __init__(self):
        self.uuid_dict = {}
        self.start_clock = dt.datetime.now()
        self.counter = 0

    def get_unitDefinition(self):
        # Define what I am
        self.unitDefinition = UnitDefinition(unitId=self.uuid_dict['PTU'],
                                         unitName='UTK PTU Unit',
                                         softwareVersion='0.1',
                                         hardwareRevision='0.1',
                                         vendor='University of Tennessee - Knoxville',
                                         unitType=UnitType.Wearable)
        return

    def initialize_uuids(self):
        # Initialize UUIDs which should remain constant.
        x = Thrift_UUID.generate_thrift_uuid()
        self.uuid_dict['PTU'] = UUID(
            leastSignificantBits=x[0],
            mostSignificantBits=x[1])
        x = Thrift_UUID.generate_thrift_uuid()
        self.uuid_dict['recordingId'] = UUID(
            leastSignificantBits=x[0],
            mostSignificantBits=x[1])
        x = Thrift_UUID.generate_thrift_uuid()
        self.uuid_dict['TemperatureSensor'] = UUID(
            leastSignificantBits=x[0],
            mostSignificantBits=x[1])
        x = Thrift_UUID.generate_thrift_uuid()
        self.uuid_dict['GPS'] = UUID(
            leastSignificantBits=x[0],
            mostSignificantBits=x[1])
        x = Thrift_UUID.generate_thrift_uuid()
        self.uuid_dict['VideoCamera'] = UUID(
            leastSignificantBits=x[0],
            mostSignificantBits=x[1])
        y = Thrift_UUID.generate_thrift_uuid()
        self.uuid_dict['VideoConfiguration'] = UUID(
            leastSignificantBits=y[0],
            mostSignificantBits=y[1])
        z = Thrift_UUID.generate_thrift_uuid()
        self.uuid_dict['VideoCameraIntrinsics'] = UUID(
            leastSignificantBits=z[0],
            mostSignificantBits=z[1])
        return

    def get_status(self):
        self.status = Status(unitId=self.uuid_dict['PTU'],
                             isRecording=True,
                             recordingId=self.uuid_dict['recordingId'],
                             hardDriveUsedPercent=0.0,  # Placeholder value
                             batteryRemainingPercent=0.0,  # Placeholder value
                             systemTime=int(time.time()))
        return

    def get_gammaDefinitions(self):
        sipmsettings = SIPMSettings(highVoltage=0.0)
        pmtsettings = PMTSettings(
            highVoltage=0.0,
            gain=0.0,
            lowerLevelDiscriminator=0.0,  # Dunno if keV or MeV
            upperLevelDiscriminator=0.0)
        sipm_pmtsettings = SIPM_PMTSettings(
            sipmSettings=sipmsettings,
            pmtSettings=pmtsettings)

        energyCalibration = []
        energyResolution = []
        dE = 3000 / 1024
        constantEnergyResolution = 0.1
        for i in range(0, 1024):
            x = EnergyCalibration(channel=i,
                                  energy=(i + 1) * dE)
            energyCalibration += [x]

            # NOTE: Constant energy resolution. In practice this is pretty stupid.
            energyResolution += [EnergyResolution(energy=x.energy,
                                                  fraction=constantEnergyResolution)]

        componentPositionAndOrientation = GridPositionAndOrientation(
            gridPosition=Vector(x=0.0, y=0.0, z=0.0),
            rotation=Quaternion(w=0.0, x=0.0, y=0.0, z=0.0))

        starting_gammaSpectrumConfig = GammaListAndSpectrumConfiguration(
            componentId=self.uuid_dict['GammaDetector'],
            settings=sipm_pmtsettings,
            energyCalibration=energyCalibration,
            componentPositionAndOrientation=componentPositionAndOrientation,
            energyResolution=energyResolution)

        component = ComponentDefinition(
            componentId=self.uuid_dict['GammaDetector'],
            componentName='GammaDetector',
            vendorName='University of Tennessee - Knoxville',
            serialNumber='00000001')
        # x=depth, width=y, length=z
        physicalDimensions = RectangularDimensions(
            depth=2 * 2.54,
            width=2 * 2.54,
            length=15.0)
        detectorMaterial = DetectorMaterial.NaI

        angularEfficiencyDefinitions = []
        for i in range(0, 1024):  # by energy
            energy = (i + 1) * dE
            angularEfficiencies = []
            for j in np.linspace(0.0, 360.0, num=37):  # by angle in 10 degree increments
                angularEfficiencies += [AngleEfficiencyPair(
                    angle=float(j) / 360.0 * 2 * np.pi,  # This is just a made up number
                    efficiency=float(j) / 360.0)]
            angularEfficiency = AngularEfficiencyDefinition(
                energy= energy,
                efficiency=angularEfficiencies)
            angularEfficiencyDefinitions += [angularEfficiency]

        self.gammaSpectrumDefinitions = [
            GammaListAndSpectrumDefinition(
                component=component,
                numberOfChannels=1024,
                physicalDimensions=physicalDimensions,
                detectorMaterial=detectorMaterial,
                startingGammaConfiguration=starting_gammaSpectrumConfig,
                angularEfficiencies=angularEfficiencyDefinitions)
        ]

        self.gammaGrossCountConfiguration = GammaGrossCountConfiguration(
            componentId=self.uuid_dict['GammaDetector'],
            componentPositionAndOrientation=componentPositionAndOrientation)

        self.gammaGrossCountDefinitions = [
            GammaGrossCountDefinition(
                component=component,
                physicalDimensions=physicalDimensions,
                detectorMaterial=detectorMaterial,
                startingGammaGrossCountConfiguration=starting_gammaGrossCountConfig,
                angularEfficiencies=angularEfficiencyDefinitions)
        ]

        # starting_gammaDoseConfig = [
        #     GammaDoseConfiguration(
        #         componentId=uuid_dict['GammaDetector'],
        #         componentPositionAndOrientation=starting_gammaConfig.componentPositionAndOrientation)
        #     ]

        # gammaDoseDefinitions = [
        #     GammaDoseDefinition(
        #         component=component,
        #         physicalDimensions=physicalDimensions,
        #         detectorMaterial=detectorMaterial,
        #         startingGammaDoseConfiguration=starting_gammaDoseConfig,
        #         angularEfficiencies=angularEfficiencies)
        #     ]

        return

    def get_environmentDefinitions(self):
        environmentalDefinitions = []

        component = ComponentDefinition(
            componentId=self.uuid_dict['GPS'],  # GPS has a temperature  sensor. However I'm not sure whether to use it or not.
            componentName='VectorNav VN-300-DEV',
            vendorName='VectorNav',
            serialNumber='100033618')

        componentPositionAndOrientation = GridPositionAndOrientation(
            gridPosition=Vector(x=10.0, y=10.0, z=10.0),
            rotation=Quaternion(w=0.0, x=0.0, y=0.0, z=0.0))
        sensorType = EnvironmentalTypes.Temperature
        self.environmentDefinitions += [
            EnvironmentalSensorDefinition(
                component=component,
                sensorType=sensorType,
                location='Somewhere',
                componentPositionAndOrientation=componentPositionAndOrientation)]

        return

    def get_navigationDefinitions(self):

        component = ComponentDefinition(
            componentId=self.uuid_dict['GPS'],
            componentName='VectorNav VN-300-DEV',
            vendorName='VectorNav',
            serialNumber='100033618')
        navOutputDefinition = NavigationOutputDefinition(
            sensorFrameOfReference=FrameOfReference.Geodetic_ECEF,
            datum=GeodeticDatum.WGS84,
            hasLatitude=True,
            hasLongitude=True,
            hasAltitude=True,
            hasX=False,  # Not sure what this means.
            hasY=False,
            hasZ=False,
            hasAccelerationX=False,  # Actually True, I just don't want to mess with this.
            hasAccelerationY=False,
            hasAccelerationZ=False,
            hasNumberOfSatellites=True,
            hasQualityOfFix=False
            hasPitch=True,
            hasRoll=True,
            hasHeading=False,  # Actually True, I just don't want to mess with this yet.
            hasVelocityX=True,
            hasVelocityY=True,
            hasVelocityZ=True,
            hasSpeed=False)

        self.navigationalDefinitions += [
            NavigationSensorDefinition(
                component=component,
                navOutputDefinition=navOutputDefinition)]
        return

    def get_contextVideoDefinitions(self):
        # contextVideoDefinitions = None
        # contextVideoConfigurations = None
        contextVideoDefinitions = []
        component = ComponentDefinition(
            componentId=self.uuid_dict['VideoCamera'],
            componentName='Logitech C615 Webcam',
            vendorName='Logitech',
            serialNumber='52A03C60')
        componentPositionAndOrientation = GridPositionAndOrientation(
            gridPosition=Vector(x=-10.0, y=-10.0, z=-10.0),
            rotation=Quaternion(w=0.0, x=0.0, y=0.0, z=0.0))

        cameraIntrinsics = CameraIntrinsics(
            componentId=self.uuid_dict['VideoCameraIntrinsics'],
            cx=int(640 / 2),
            cy=int(480 / 2),
            fx=640,
            fy=480)

        contextVideoConfigurations = ContextVideoConfiguration(
            componentId = self.uuid_dict['VideoConfiguration'],
            fileName='NANI.avi',
            framesPerSecond=30.0,
            verticalResolution=480,
            horizontalResolution=640,
            componentPositionAndOrientation=componentPositionAndOrientation,
            verticalFOV=48.7,
            horizontalFOV=62.2,
            isRectified=False,  # Correlating perspective from multiple spatial positions. Should be true for stereo cameras
            intrinsics=cameraIntrinsics,
            isDeBayered=False,  # Partial colors converted into full colors
            timeStamp=int(time.time() * 1000))
        # # NOTE: there's also "intrinsics" if isRectified is False.
        contextVideoDefinitions += [
            ContextVideoDefinition(
                component=component,
                videoConfiguration=contextVideoConfigurations)]
        return contextVideoDefinitions, contextVideoConfigurations

    def get_systemDefinition(self):
        # Defining systemDefinition

        gammaSpectrumDefinitions, gammaGrossCountDefinitions = \
            self.get_gammaDefinitions()
        environmentDefinitions = get_environmentDefinitions()
        navigationDefinitions = get_navigationDefinitions()
        # contextVideoDefinition = get_contextVideoDefinitions()

        systemDefinition = SystemDefinition(
            gammaSpectrumDefinitions=gammaSpectrumDefinitions,
            gammaGrossCountDefinitions=gammaGrossCountDefinitions,
            environmentalDefinitions=environmentDefinitions,
            navigationSensorDefinitions=navigationDefinitions,
            timeStamp=int(time.time() * 1000),
            apiVersion='yolo')

        return systemDefinition

    def construct_configurations(self, x, y):
        out = []
        if x is not None:
            for i in range(len(x)):
                out += [getattr(x[i], y)]
        else:
            out = None
        return out

    def get_systemConfiguration(self, unitDefinition, systemDefinition):
        gammaSpectrumConfigurations = self.construct_configurations(
            systemDefinition.gammaSpectrumDefinitions,
            'startingGammaConfiguration')
        gammaListConfigurations = self.construct_configurations(
            systemDefinition.gammaListDefinitions,
            'startingGammaConfiguration')
        gammaGrossCountConfigurations = self.construct_configurations(
            systemDefinition.gammaGrossCountDefinitions,
            'startingGammaGrossCountConfiguration')
        # gammaDoseConfigurations = construct_configurations(
        #     systemDefinition.gammaDoseDefinitions,
        #     'startingGammaDoseConfiguration')

        neutronListConfigurations = self.construct_configurations(
            systemDefinition.neutronListDefinitions,
            'startingNeutronListConfiguration')
        neutronSpectrumConfigurations = self.construct_configurations(
            systemDefinition.neutronSpectrumDefinitions,
            'startingNeutronSpectrumConfiguration')
        neutronGrossCountConfigurations = self.construct_configurations(
            systemDefinition.neutronGrossCountDefinitions,
            'startingNeutronGrossCountConfiguration')

        # NOTE: Skipping most of the video stuff for now.
        contextVideoConfigurations = self.construct_configurations(
            systemDefinition.contextVideoDefinitions,
            'videoConfiguration')
        # contextPointCloudConfigurations = construct_configurations(
        #     systemDefinition.contextPointCloudDefinitions,
        #     'startingPointCloudConfiguration')
        # contextVoxelConfigurations = construct_configurations(
        #     systemDefintion.contextVoxelDefinitions,
        #     'startingVoxelCon

        # NOTE: this is also blank. Structure is not quite defined as of yet.
        # algorithmConfigurations = construct_configurations(
        #     systemDefinition.

        # passiveMaterialConfigurations = systemDefinition.passiveMaterialDefinitions

        # contextStreamConfigurations = \
        #     systemDefinition.contextStreamDefinitions.configuration

        self.systemConfiguration = SystemConfiguration(
            unitId=unitDefinition.unitId,
            gammaSpectrumConfigurations=gammaSpectrumConfigurations,
            gammaListConfigurations=gammaListConfigurations,
            gammaGrossCountConfigurations=gammaGrossCountConfigurations,
            neutronListConfigurations=neutronListConfigurations,
            neutronSpectrumConfigurations=neutronSpectrumConfigurations,
            neutronGrossCountConfigurations=neutronGrossCountConfigurations,
            contextVideoConfigurations=contextVideoConfigurations)
        return

    def get_recordingUpdate(self):

        self.recordingConfig = RecordingConfiguration(
            unitId=self.uuid_dict['PTU'],
            recordingId=self.uuid_dict['recordingId'],
            campaign='Test',
            tag='yolo',
            description='Fake PTU Testing to make sure the CVRS is working as intended',
            location='Somewhere',
            fileName='PTU_local.sqlite3',
            recordingType=RecordingType.Measurement,
            recordingDuration=0,
            POSIXStartTime=int(time.time()),
            measurementNumber=1)

        self.recordingUpdate = RecordingUpdate(recordingConfig=self.recordingConfig)
        return


    def initialize_measurement_thread(self, filenames, outputdbo):
        # This should only be called once.

        self.gammaHandling = []
        self.threads = []
        for i in range(len(filenames)):
            self.gammaHandling += [CSVSeeker(filenames[i], i)]

        # Initialize the GPS handler and the video handler here.
        # VN-300 stuff
        self.gps = VnSensor()
        print('Connecting to GPS sensor')
        self.gps.connect('/dev/ttyUSB0', 115200)
        print('Connected to GPS?: ', self.gps.is_connected)
        self.environment_sensor = self.gps
        #######

        # initialize the payload
        self.payload = []
        return


    def measurement_spool(self):
        time.sleep(1)
        c = time.time()
        # Construct thrift objects for packaging.
        # 3) pushData
        # NOTE: Decision: I'll control frequency from here. CSVSeeker has to keep track of the index and read from that point on. This will make it more modular and when I decide to update the spool to use the CAENLibs it'll be easier to do so.
        # NOTE: Decision: I'll package, send, then dump.

        # QUESTION: How do I call this function???

        # gammaSpec = []
        # gammaCounts = []
        livetime = 1.0  # HACK
        realtime = 1.0  # HACK
        gammaSpectrumData = []
        gammaGrossCountData = []
        for i in range(len(self.gammaHandling)):
            snapshot = self.gammaHandling[i].get_updates()
            # gammaSpec += [snapshot]
            # gammaCounts += [np.sum(snapshots)]
            intSpectrum = [int(x) for x in snapshot]
            integerSpectrum = Spectrum(
                spectrumInt=intSpectrum,
                format=SpectrumFormat.ARRAY,
                channelCount=len(intSpectrum),
                liveTime=livetime)

            spectrumResult = SpectrumResult(
                intSpectrum=integerSpectrum)

            gammaSpectrumData += [GammaSpectrumData(
                componentId=self.uuid_dict['GammaDetector'],
                timeStamp=timestamp,
                health=Health.Nominal,
                spectrum=spectrumResult,
                liveTime=livetime,
                realTime=realtime)]
            gammaGrossCrossData += [GammaGrossCountData(
                componentId=self.uuid_dict['GammaDetector'],
                timeStamp=int(time.time()),
                health=Health.Nominal,
                counts=sum(intSpectrum),
                liveTime=1.0,
                realTime=1.0)]

        # gammaSpectrumData = get_gammaSpectrumData(db, lasttime)
        # gammaSpectrumData = self.get_gammaSpectrumData()
        # gammaListData  = get_gammaListData(db)
        gammaListData = []
        # gammaGrossCountData = get_gammaGrossCountData(db, lasttime)
        # gammaGrossCountData = self.get_gammaGrossCountData()
        # gammaDoseData = get_gammaDoseData(db)
        gammaDoseData = []
        # neutronListData = get_neutronListData(db)
        neutronListData = []
        # neutronSpectrumData = get_neutronSpectrumData(db)
        neutronSpectrumData = []
        # neutronGrossCountData = get_neutronGrossCountData(db)
        neutronGrossCountData = []
        navigationData = self.get_navigationData()  # GPS data
        # navigationData = []
        environmentalData = self.get_environmentalData()  # Temperature and pressure data from the GPS module
        # environmentalData = []
        # videoData = get_videoData(db)  # Let's, leave this for now I think.
        videoData = []
        pointCloudData = []
        voxelData = []
        meshData = []
        messages = []
        waypoints = []
        boundingboxes = []
        markers = []
        algorithmData = []
        streamIndexData = []
        configuration = [systemConfiguration]

        dataPayload = DataPayload(
            unitId=self.uuid_dict['PTU'],
            timeStamp=int(time.time() * 1000),
            systemHealth=Health.Nominal,
            isEOF=False,
            # recordingConfig=recordingConfiguration,
            gammaSpectrumData=gammaSpectrumData,
            # gammaListData=gammaListData,
            gammaGrossCountData=gammaGrossCountData,
            # gammaDoseData=gammaDoseData,
            # neutronListData=neutronListData,
            # neutronSpectrumData=neutronSpectrumData,
            # neutronGrossCountData=neutronGrossCountData,
            environmentalData=environmentalData,
            navigationData=navigationData)
            # videoData=videoData,
            # pointCloudData=pointCloudData,
            # voxelData=voxelData,
            # meshData=meshData,
            # messages=messages,
            # waypoints=waypoints,
            # boundingBoxes=boundingboxes,
            # markers=markers,
            # algorithmData=algorithmData,
            # streamIndexData=streamIndexData,
            # configuration=configuration)
        d = time.time()
        print('Payload construction {}s'.format(d-c))
        c = time.time()

        self.payload += [dataPayload]

        # Write the data in the thrift package into the sqlite3 on PTU local.
        # Write to db every second? Or write everytime I push????
        # self.db.stack_datum(singlePayload)
        return

    def get_navigationData(self):
        # ypr = self.gps.read_yaw_pitch_roll()  # Should be a 3v
        ecef_register = self.gps.read.read_gps_solution_ecef()  # ECEF register

        location = Location(
            latitude=ecef_register.position.x,
            longitude=ecef_register.position.y,
            altitude=ecef_register.position.z)


        # 1: UUID.UUID waypointId;
        # 2: i64 timeStamp; // POSIX time * 1000 - should match data packet's time stamp
        # 3: string name;
        # 4: Location location;
        x = Thrift_UUID.generate_thrift_uuid()
        uuid = UUID(
            leastSignificantBits=x[0],
            mostSignificantBits=x[1])
        data = Waypoint(
            waypointId=uuid,
            timeStamp=int(time.time()) * 1000,
            name='measurement_waypoint',
            location=location)

        return data

    def get_environmentData(self):

        register = self.environment_sensor.read_imu_measurements()
        temp_data = register.temp
        x = Thrift_UUID.generate_thrift_uuid()
        uuid = UUID(
            leastSignificantBits=x[0],
            mostSignificantBits=x[1])
        data = EnvironmentalSensorData(
            component=self.uuid_dict['GPS'],
            timeStamp=int(time.time()) * 1000,
            health=Health.Nominal,
            value=temp_data)

        return data

    def main_loop(self):

        # Make socket
        transport = TSocket.TSocket('127.0.0.1', 9090)

        # Buffering is critical. Raw sockets are very slow
        transport = TTransport.TBufferedTransport(transport)

        # Wrap in a protocol
        protocol = TBinaryProtocol.TBinaryProtocol(transport)

        # Create a client to use the protocol encoder
        # client = CVRSHandler.Client(protocol)
        # client = Client(protocol)
        client = CVRSServices.CVRSEndpoint.Client(protocol)

        # Connect!
        transport.open()

        initialize_uuids()

        unit_definition = get_unitDefinition()
        # Initiate handshake
        session = client.registerPtu(unitDefinition=unit_definition)
        # time.sleep(5)  # this method returns a session class from CVRSServices

        status = get_initialStatus()
        systemDefinition = get_systemDefinition()
        systemConfiguration = get_systemConfiguration(unit_definition, systemDefinition)
        recordingUpdate, recordingConfiguration = get_recordingUpdate()

        ptu_message = client.define(session.sessionId, status,
                                    systemDefinition, systemConfiguration,
                                    recordingUpdate)

        self.db = DatabaseOperations('./PTU_local.sqlite3')
        self.db.initialize_structure(numdetectors=4)

        counter = 0
        status = self.get_initialStatus()

        # gammaFilenames = [\
        #     'det_0.csv',
        #     'det_1.csv',
        #     'det_2.csv',
        #     'det_3.csv']
        gammaFilenaes = [\
            '0@DT5720D #2-3-1167_Data_daq_test.csv',
            '1@DT5720D #2-3-1167_Data_daq_test.csv',
            '2@DT5720D #2-3-1167_Data_daq_test.csv',
            '3@DT5720D #2-3-1167_Data_daq_test.csv']

        self.initialize_measurement_thread(gammaFilenames)
        # QUESTION: Start threads threads here, they populate self.package.
        # self.spool_measurement_thread()
        # NOTE: I may have to de classify spool_measurement_thread, why may be tricky since it needs the self.payload attribute.
        self.thread = Thread(target=self.measurement_spool,
                             name='measurement_packaging')
        self.thread.start()

        while True:
            # NOTE: There should be a sleep for one second for each independent process.
            time.sleep(1.0)  # Sleep for one second
            a = time.time()
            # 1) Look for  file changes and dump to the database
            # 2) report the status (should be the same message)
            # definitionAndConfigurationUpdate = DefinitionAndConfigurationUpdate(
            #     systemDefinition=systemDefinition,
            #     systemConfiguration=systemConfiguration)
            definitionAndConfigurationUpdate = DefinitionAndConfigurationUpdate()

            c = time.time()
            isGood = client.reportStatus(
                sessionId=session.sessionId,
                status=status,
                definitionAndConfigurationUpdate=definitionAndConfigurationUpdate)
            d = time.time()
            print('Report Status {}s'.format(d - c))

            isGood = client.pushData(
                sessionId=session.sessionId,
                datum=self.payload,
                definitionAndConfigurationUpdate=definitionAndConfigurationUpdate)
            d = time.time()
            print('Payload delivery {}s'.format(d-c))
            while not isGood:
                print('delivery failure')
                # not sure if sleep is good here. or continuous trying
                isGood = client.pushData(
                    sessionId=session.sessionId,
                    datum=self.payload,
                    definitionAndConfigurationUpdate=definitionAndConfigurationUpdate)

            # QUESTION: Write everytime I stack into PTU Local? Or write everytime I push?

            # Empty the payload buffer
            self.payload = []

            acks = []
            # NOTE: No way of handling acks right now.
            # 4) pushAcknowledgements
            isGood = client.pushAcknowledgements(
                sessionId=session.sessionId,
                acknowledgements=acks)
            b = time.time()
            print('time elapsed: {}'.format(b-a))
            lasttime = time.time()



        # NOTE: Frankly I should never get to the close, since I'll be infinitely looping
        transport.close()
        return


def main():
    try:
        ptu = PTU()
        ptu.main_loop()
    except Thrift.TException as tx:  # catch the thrift exceptions
        print('%s' % tx.message)
    return


main()
