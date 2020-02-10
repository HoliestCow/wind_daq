
############### PTU STUFF ###############
# for getting data out of the CAEN digitizer
# import caenlib

# Class for getting data out of Compass CSVs
from wind_daq.data_acquisition.ptu.libs.compass.readout import CompassReadout

import CVRSServices.CVRSEndpoint
from PTUPayload.ttypes import (UnitDefinition, UnitType, Status,
                               SystemDefinition, SystemConfiguration,
                               RecordingConfiguration, RecordingType,
                               DataPayload)
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
                                     GeodeticDatum,
                                     NavigationData)
from Navigation.ttypes import Location
from EnvironmentalSensor.ttypes import (EnvironmentalTypes,
                                        EnvironmentalSensorDefinition,
                                        EnvironmentalSensorData)
from ContextSensor.ttypes import (ContextVideoConfiguration,
                                  ContextVideoDefinition,
                                  CameraIntrinsics,
                                  ContextStreamConfiguration,
                                  ContextStreamDefinition,
                                  ContextStreamIndexData,
                                  SensorModalityConfiguration)
from Spectrum.ttypes import (SpectrumResult, Spectrum,
                             SpectrumFormat)
from Health.ttypes import (Health)

from UUID.ttypes import UUID

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

import time
import datetime as dt

############ Database Stuff ##############

from wind_daq.data_acquisition.utils.thrift_uuid import Thrift_UUID
from wind_daq.data_acquisition.utils.database import DatabaseOperations
from wind_daq.data_acquisition.utils.camera import CameraStream
import numpy as np

######### VN 300 Libraries ##########
from wind_daq.data_acquisition.ptu.libs.vectornav.vnpy import *

##### Other utilities

# from threading import Thread
from wind_daq.data_acquisition.utils.threads import StoppableThread

########## DONE IMPORTING ################


class PTU:
    def __init__(self, dataDir):
        self.uuid_dict = {}
        self.start_clock = dt.datetime.now()
        self.counter = 0
        self.dataDir = dataDir

    def initialize_unitDefinition(self):
        # Define what I am
        self.unitDefinition = UnitDefinition(unitId=self.uuid_dict['PTU'],
                                             unitName='UTK PTU Unit',
                                             softwareVersion='0.1',
                                             hardwareRevision='0.1',
                                             vendor='University of Tennessee - Knoxville',
                                             unitType=UnitType.Wearable)
        return

    def get_time(self):
        now = dt.datetime.now()
        epoch = dt.datetime(year=1970, month=1, day=1)
        timesinceepoch = int((now-epoch).total_seconds())
        return timesinceepoch

    def initialize_uuids(self):
        # Initialize UUIDs which should remain constant.
        x = Thrift_UUID.generate_thrift_uuid()
        self.uuid_dict['PTU'] = UUID(
            leastSignificantBits=x[0],
            mostSignificantBits=x[1])
        x = Thrift_UUID.generate_thrift_uuid()
        self.uuid_dict['GammaDetector'] = UUID(
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
                             systemTime=self.get_time())
        return

    def get_gammaDefinitions(self):
        self.numEnergyChannels = 4096

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
        dE = 3000 / self.numEnergyChannels
        constantEnergyResolution = 0.1
        for i in range(0, 1024):
            x = EnergyCalibration(channel=i,
                                  energy=(i + 1) * dE)
            energyCalibration += [x]

            # NOTE: Constant energy resolution. In practice this is dumb.
            energyResolution += [EnergyResolution(energy=x.energy,
                                                  fraction=constantEnergyResolution)]

        componentPositionAndOrientation = GridPositionAndOrientation(
            gridPosition=Vector(x=0.0, y=0.0, z=0.0),
            rotation=Quaternion(w=0.0, x=0.0, y=0.0, z=0.0))

        gammaSpectrumConfig = GammaListAndSpectrumConfiguration(
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
            for j in np.linspace(0.0, 360.0, num=37):
                # by angle in 10 degree increments
                # this efficiency stuff is made up, but this is how we
                # encode the angular response of the system itself.
                angularEfficiencies += [AngleEfficiencyPair(
                    angle=float(j) / 360.0 * 2 * np.pi,
                    efficiency=float(j) / 360.0)]
            angularEfficiency = AngularEfficiencyDefinition(
                energy=energy,
                efficiency=angularEfficiencies)
            angularEfficiencyDefinitions += [angularEfficiency]

        # self.gammaSpectrumDefinitions = 4 * [
        #     GammaListAndSpectrumDefinition(
        #         component=component,
        #         numberOfChannels=1024,  # TODO: Check this number. It might be 16s and 8s
        #         physicalDimensions=physicalDimensions,
        #         detectorMaterial=detectorMaterial,
        #         startingGammaConfiguration=gammaSpectrumConfig,
        #         angularEfficiencies=angularEfficiencyDefinitions)]

        # This is 2 16k channel dets for the NaI and 2 8k channels for the CLYC
        # TODO: Measure and input the proper angular efficiencies.
        self.gammaSpectrumDefinitions = []
        self.gammaSpectrumDefinitions += 2 * [
                GammaListAndSpectrumDefinition(
                    component=component,
                    numberOfChannels=2**14,
                    physicalDimensions=physicalDimensions,
                    detectorMaterial=detectorMaterial,
                    startingGammaConfiguration=gammaSpectrumConfig,
                    angularEfficiencies=angularEfficiencyDefinitions)]
        self.gammaSpectrumDefinitions += 2 * [
                GammaListAndSpectrumDefinition(
                    component=component,
                    numberOfChannels=2**13,
                    physicalDimensions=physicalDimensions,
                    detectorMaterial=detectorMaterial,
                    startingGammaConfiguration=gammaSpectrumConfig,
                    angularEfficiencies=angularEfficiencyDefinitions)]


        gammaGrossCountConfiguration = GammaGrossCountConfiguration(
            componentId=self.uuid_dict['GammaDetector'],
            componentPositionAndOrientation=componentPositionAndOrientation)

        # This should hold for both the original place holder config and the 2 NAI 2 CLYC case.
        self.gammaGrossCountDefinitions = 4 * [
            GammaGrossCountDefinition(
                component=component,
                physicalDimensions=physicalDimensions,
                detectorMaterial=detectorMaterial,
                startingGammaGrossCountConfiguration=gammaGrossCountConfiguration,
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
        self.environmentDefinitions = []
        # GPS has a temperature  sensor
        component = ComponentDefinition(
            componentId=self.uuid_dict['GPS'],
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

        self.navigationDefinitions = []

        component = ComponentDefinition(
            componentId=self.uuid_dict['GPS'],
            componentName='VectorNav_VN-300-DEV',
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
            hasQualityOfFix=False,
            hasPitch=True,
            hasRoll=True,
            hasHeading=False,  # Actually True, I just don't want to mess with this yet.
            hasVelocityX=True,
            hasVelocityY=True,
            hasVelocityZ=True,
            hasSpeed=False)

        self.navigationDefinitions += [
            NavigationSensorDefinition(
                component=component,
                navOutputDefinition=navOutputDefinition)]
        return

    def get_contextVideoDefinitions(self):
        # contextVideoDefinitions = None
        # contextVideoConfigurations = None
        self.contextVideoDefinitions = []
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
        self.contextVideoConfigurations = ContextVideoConfiguration(
            componentId=self.uuid_dict['VideoConfiguration'],
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
            timeStamp=self.get_time())
        # # NOTE: there's also "intrinsics" if isRectified is False.
        self.contextVideoDefinitions += [
            ContextVideoDefinition(
                component=component,
                videoConfiguration=self.contextVideoConfigurations)]
        return

    def get_videostreamingDefinitions(self):
        # fills in self.streamDefinition.
        streamingdefinition_template = self.contextVideoDefinitions[0]
        streamingconfiguration_template = self.contextVideoConfigurations
        modalityconfiguration = SensorModalityConfiguration(
            videoConfiguration=streamingconfiguration_template)

        self.uuid_dict['yolo_stream'] = Thrift_UUID.generate_thrift_uuid()
        self.uuid_dict['siamfc_stream'] = Thrift_UUID.generate_thrift_uuid()

        self.contextStreamConfiguration = []

        self.contextStreamConfiguration += [ContextStreamConfiguration(
            componentId=self.uuid_dict['yolo_stream'],
            componentPositionAndOrientation=streamingconfiguration_template.componentPositionAndOrientation,
            modalityConfiguration=modalityconfiguration,
            timeStamp=self.get_time())]
        # self.contextStreamConfiguration += [ContextStreamConfiguration(
        #     componentId=self.uuid_dict['siamfc_stream'],
        #     componentPositionAndOrientation=streamingconfiguration_template.componentPositionAndOrientation,
        #     modalityConfiguration=modalityconfiguration,
        #     timeStamp=int(time.time()))]

        self.contextStreamDefinition = []
        self.contextStreamDefinition += [ContextStreamDefinition(
            component=streamingdefinition_template.component,
            streamFormat=None,
            streamAddress='./images/yolo/',
            formatVersion=None,
            documentationURI=None,
            configuration=self.contextStreamConfiguration[0])]
        # self.contextStreamDefinition += [ContextStreamDefinition(
        #     component=streamingdefinition_template.component,
        #     streamFormat=None,
        #     streamAddress='./images/siamfc/',
        #     formatVersion=None,
        #     documentationURI=None,
        #     configuration=self.contextStreamConfiguration[1])]
        return

    def get_systemDefinition(self):
        # Defining systemDefinition

        self.get_gammaDefinitions()
        # below is for the temperature sensor on the VN 300. Skip for now.
        # self.get_environmentDefinitions()
        # below works, I was just testing just the gamma stuf
        # self.get_navigationDefinitions()

        # I have to call get_contextVideo before videostreaming since it
        # initializes stuff I need.
        self.get_contextVideoDefinitions()
        self.get_videostreamingDefinitions()

        self.systemDefinition = SystemDefinition(
            gammaSpectrumDefinitions=self.gammaSpectrumDefinitions,
            gammaGrossCountDefinitions=self.gammaGrossCountDefinitions,
            # environmentalDefinitions=self.environmentDefinitions,
            # navigationSensorDefinitions=self.navigationDefinitions,
            # contextVideoDefinitions=self.contextVideoDefinitions,
            timeStamp=self.get_time(),
            apiVersion='yolo')
            # contextStreamDefinitions=self.contextStreamDefinition) # this is fucked up. For some reason there's a tuple inside here.

        return

    def construct_configurations(self, x, y):
        out = []
        if x is not None:
            for i in range(len(x)):
                out += [getattr(x[i], y)]
        else:
            out = None
        return out

    def get_systemConfiguration(self):
        gammaSpectrumConfigurations = self.construct_configurations(
            self.systemDefinition.gammaSpectrumDefinitions,
            'startingGammaConfiguration')
        gammaListConfigurations = self.construct_configurations(
            self.systemDefinition.gammaListDefinitions,
            'startingGammaConfiguration')
        gammaGrossCountConfigurations = self.construct_configurations(
            self.systemDefinition.gammaGrossCountDefinitions,
            'startingGammaGrossCountConfiguration')
        # gammaDoseConfigurations = construct_configurations(
        #     systemDefinition.gammaDoseDefinitions,
        #     'startingGammaDoseConfiguration')

        neutronListConfigurations = self.construct_configurations(
            self.systemDefinition.neutronListDefinitions,
            'startingNeutronListConfiguration')
        neutronSpectrumConfigurations = self.construct_configurations(
            self.systemDefinition.neutronSpectrumDefinitions,
            'startingNeutronSpectrumConfiguration')
        neutronGrossCountConfigurations = self.construct_configurations(
            self.systemDefinition.neutronGrossCountDefinitions,
            'startingNeutronGrossCountConfiguration')

        # NOTE: Skipping most of the video stuff for now.
        contextVideoConfigurations = self.construct_configurations(
            self.systemDefinition.contextVideoDefinitions,
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
            unitId=self.unitDefinition.unitId,
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
            POSIXStartTime=self.get_time(),
            measurementNumber=1)

        self.recordingUpdate = RecordingUpdate(recordingConfig=self.recordingConfig)
        return

    def caenlib_spool(self):
        num_caen_channels = 4
        num_channels = 4096

        self.gammaHandlingState = np.array([1], dtype=np.int32)
        self.gammaHandlingData = np.zeros((num_caen_channels, num_channels), dtype=np.uint32)

        # Here we make the cfunction call to sit on a single threadself.
        # The only input is a pointer to the integer which describes what state we'd like the thread to be in.
        # input:
        # state.
            # 0 = idle
            # 1 = start acquisition
            # 2 = stop acquisition
            # 3 = cleanup and jump out of the code.
        self.caenlib_thread = StoppableThread(
            target=caenlib.measurement_spool,
            args=(self.gammaHandlingState,))
        self.caenlib_thread.start()
        time.sleep(10)
        print('started caenlib spool')
        # t1.join()
        # NOTE: Have to test by setting self.gammaHandlingState to not zero and check in C.
        # Make sure it's reading from the pointer directly in each loop.

    def get_gammaData(self):
        # gammaSpec = []
        # gammaCounts = []
        livetime = 1000  # HACK
        realtime = 1000  # HACK
        gammaSpectrumData = []
        gammaGrossCountData = []
        # for i in range(len(self.gammaHandling)):
        # for i in range(self.gammaHandlingData.shape[0]):
        for i in range(len(self.systemDefinition.gammaSpectrumDefinitions)):
            # NOTE: I have to figure out how this exactly works. And surely there's a step to convert long and short charge integrations into energy deposited. But I'm not sure how this is supposed to work. More testing :/

            # NOTE: why do I do this??
            #############################################
            # snapshot = self.gammaHandling[i].get_updates()
            # gammaSpec += [self.gammaHandlingLongData]
            # gammaCounts += [np.sum(self.gammaHandlingLongData)]
            # gammaSpec += [snapshot]
            # gammaCounts += [np.sum(snapshots)]
            #############################################
            print('before intSpec')
            # intSpectrum = [int(x) for x in self.gammaHandlingData[i, :]]

            # HACK: This is to fix that offset issue since CHannel 2 is dead.
            if str(i) in self.readout.current_measurement:
                intSpectrum = [int(x) for x in self.readout.current_measurement[str(i)]['energy_spectrum']]
            else:
                intSpectrum = [int(x) for x in self.readout.current_measurement[str(i + 1)]['energy_spectrum']]
            print(sum(intSpectrum))
            print('after intSpec')
            integerSpectrum = Spectrum(
                spectrumInt=intSpectrum,
                format=SpectrumFormat.ARRAY,
                channelCount=len(intSpectrum),
                liveTime=livetime)

            spectrumResult = SpectrumResult(
                intSpectrum=integerSpectrum)

            gammaSpectrumData += [GammaSpectrumData(
                componentId=self.uuid_dict['GammaDetector'],
                timeStamp=self.get_time(),
                health=Health.Nominal,
                spectrum=spectrumResult,
                liveTime=livetime,
                realTime=realtime)]
            gammaGrossCountData += [GammaGrossCountData(
                componentId=self.uuid_dict['GammaDetector'],
                timeStamp=self.get_time(),
                health=Health.Nominal,
                counts=int(sum(intSpectrum[100:])),
                liveTime=livetime,
                realTime=realtime)]
            print(sum(intSpectrum))
        return gammaSpectrumData, gammaGrossCountData

    def get_streamIndexData(self):
        streamTime = self.get_time() 
        x = []
        for key in self.camera:
            x += [ContextStreamIndexData(
                componentId=self.uuid_dict[key],
                timestamp=self.get_time(),
                # streamTimeStamp=streamTime)]  # not exactly right. We should
                # grab this from the index from the camera stream class.
                streamTimeStamp=self.camera[key].image_index)]
        return x

    def measurement_spool(self, measurement_time):
        print('starting  measurement spool')
        # for yolo in range(measurement_time):
        self.readout = CompassReadout(self.dataDir)
        for i in range(measurement_time):
            if self.payload_thread.stopped():
                break
            time.sleep(1)
            # caenlib.update_histograms(self.gammaHandlingData)
            self.readout.update_measurement()
            c = time.time() 
            # Construct thrift objects for packaging.
            # 3) pushData
            # NOTE: Decision: I'll control frequency from here. CSVSeeker has to keep track of the index and read from that point on. This will make it more modular and when I decide to update the spool to use the CAENLibs it'll be easier to do so.
            # NOTE: Decision: I'll package, send, then dump.
            gammaSpectrumData, gammaGrossCountData = self.get_gammaData()
            # streamIndexData = self.get_streamIndexData()

            gammaListData = []
            gammaDoseData = []
            neutronListData = []
            neutronSpectrumData = []
            neutronGrossCountData = []

            navigationData = []
            # navigationData = [self.get_navigationData()]
            # environmentalData = self.get_environmentalData()  # Temperature and pressure data from the GPS module
            videoData = []
            pointCloudData = []
            voxelData = []
            meshData = []
            messages = []
            waypoints = []
            boundingboxes = []
            markers = []
            algorithmData = []
            # streamIndexData = self.streamIndexData
            streamIndexData = []
            configuration = [self.systemConfiguration]

            dataPayload = DataPayload(
                unitId=self.uuid_dict['PTU'],
                timeStamp=self.get_time(),
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
                # environmentalData=environmentalData,
                # navigationData=navigationData)
                # videoData=videoData,
                # pointCloudData=pointCloudData,
                # voxelData=voxelData,
                # meshData=meshData,
                # messages=messages,
                # waypoints=waypoints,
                # boundingBoxes=boundingboxes,
                # markers=markers,
                # algorithmData=algorithmData,
                streamIndexData=streamIndexData)
                # configuration=configuration)
            d = time.time()
            print('Payload construction {}s'.format(d-c))
            c = time.time()

            self.payload = dataPayload

            # Write the data in the thrift package into the sqlite3 on PTU local.
            # Write to db every second? Or write everytime I push????
            # self.db.stack_datum(singlePayload)
        return

    def get_navigationData(self):
        print('polling vectornav')
        # ypr = self.gps.read_yaw_pitch_roll()  # Should be a 3v
        lla_register = self.gps.read_gps_solution_lla()  # ECEF register

        location = Location(
            latitude=float(lla_register.lla.x),
            longitude=float(lla_register.lla.y),
            altitude=float(lla_register.lla.z))
        # location = Vector(
        #     x=float(ecef_register.position.x),
        #     y=float(ecef_register.position.y))

        # NOTE: GPS data has to be in a list
        data = NavigationData(
            componentId=self.uuid_dict['GPS'],
            timeStamp=self.get_time(),
            location=location,
            # position=location,
            health=Health.Nominal)

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
            timeStamp=self.get_time(),
            health=Health.Nominal,
            value=temp_data)

        return data

    def start_gps(self):
        self.gps = VnSensor()
        self.gps.connect('/dev/ttyUSB0', 115200)

    def main_loop(self):

        # Make socket
        # self.transport = TSocket.TSocket('10.130.130.118', 8080)
        self.transport = TSocket.TSocket('localhost', 8080)

        # Buffering is critical. Raw sockets are very slow
        self.transport = TTransport.TBufferedTransport(self.transport)

        # Wrap in a protocol
        protocol = TBinaryProtocol.TBinaryProtocol(self.transport)

        # Create a client to use the protocol encoder
        # client = CVRSHandler.Client(protocol)
        # client = Client(protocol)
        client = CVRSServices.CVRSEndpoint.Client(protocol)

        # Connect!
        self.transport.open()

        self.initialize_uuids()

        self.initialize_unitDefinition()
        # Initiate handshake
        print('Initiating handhsake')
        session = client.registerPtu(unitDefinition=self.unitDefinition)
        print('Shook hands')
        # time.sleep(5)  # this method returns a session class from CVRSServices

        self.get_status()
        self.get_systemDefinition()
        self.get_systemConfiguration()
        self.get_recordingUpdate()
        print('defininig')
        ptu_message = client.define(session.sessionId, self.status,
                                    self.systemDefinition, self.systemConfiguration,
                                    self.recordingUpdate)
        print('defined')
        print('initializing db')
        self.db = DatabaseOperations('./PTU_local.sqlite3')
        self.db.initialize_structure(self.systemDefinition)

        # starting gamma sensor services.
        measurement_time = 300
        self.payload = None
        # self.gammaHandlingState[0] = 1  # start acquisition. on the clib side

        # NOTE: Payload thread should sit another spool that happens every 1 second. Measurement spool should have a while true I think.

        self.payload_thread = StoppableThread(
            target=self.measurement_spool,
            args=(measurement_time,),
            name='payload_spool')
        self.payload_thread.start()

        # starting navigation services.
        # self.start_gps()

        # start camera acquisition
        self.camera = {}
        self.camera['yolo_stream'] = CameraStream(
            src=0,
            fps=1,
            file_prefix='./images/raw/')

        self.camera['yolo_stream'].start()
        timetosleep = 0
        for lol in range(measurement_time):
            # NOTE: There should be a sleep for one second for each independent process.
            # NOTE: Fixed time.sleep messes with the payload. Needs to be variable time.sleep().I assume each process takes less than one second.
            print('sleeping for: ', timetosleep)
            if timetosleep > 0:
                time.sleep(timetosleep)  # Sleep for one second
            a = time.time()
            # 1) Look for  file changes and dump to the database
            # 2) report the status (should be the same message)
            # definitionAndConfigurationUpdate = DefinitionAndConfigurationUpdate(
            #     systemDefinition=systemDefinition,
            #     systemConfiguration=systemConfiguration)
            definitionAndConfigurationUpdate = DefinitionAndConfigurationUpdate()

            c = time.time()
            messagefromcvrs = client.reportStatus(
                sessionId=session.sessionId,
                status=self.status,
                definitionAndConfigurationUpdate=definitionAndConfigurationUpdate)
            d = time.time()
            print('Report Status {}s'.format(d - c))
            # print(self.payload)
            print('trying to pushdata')
            isGood = client.pushData(
                sessionId=session.sessionId,
                datum=self.payload,
                definitionAndConfigurationUpdate=definitionAndConfigurationUpdate)
            # time.sleep(5)
            print(isGood)
            print('imadeit???')
            print('called the function')
            d = time.time()
            print(isGood)
            while not isGood:
                print('delivery failure')
                # not sure if sleep is good here. or continuous trying
                isGood = client.pushData(
                    sessionId=session.sessionId,
                    datum=self.payload,
                    definitionAndConfigurationUpdate=definitionAndConfigurationUpdate)

            if isGood:
                # push data into own PTU_local
                self.db.stack_datum(self.payload)
            print('Payload delivery {}s'.format(d - c))

            # QUESTION: Write everytime I stack into PTU Local? Or write everytime I push?

            # Empty the payload buffer
            self.payload = None

            acks = []
            # NOTE: No way of handling acks right now.
            # 4) pushAcknowledgements
            isGood = client.pushAcknowledgements(
                sessionId=session.sessionId,
                acknowledgements=acks)
            b = time.time()
            print('time elapsed: {}'.format(b - a))
            timetosleep = 1 - (b - a)
        # NOTE: Frankly I should never get to the close, since I'll be infinitely looping
        # self.gammaHandlingState[0] = 3
        time.sleep(1)
        self.caenlib_thread.stop()
        self.payload_thread.stop()
        self.transport.close()
        print('getting out of main_loop')
        return


def ptu_cleanup(ptu_object):
    ptu_object.gammaHandlingState = 3
    ptu_object.transport.close()
    time.sleep(1)
    return


def main():
    try:
        # dataDir = './'  # path to where the data is being dumped toa
        dataDir = '/home/daq/Documents/caen_drivers_programs/CoMPASS-1.3.0/projects/wind/DAQ/run/UNFILTERED/'  # This is from the default run thing
        ptu = PTU(dataDir)
        ptu.main_loop()
    except Thrift.TException as tx:  # catch the thrift exceptions
        print('%s' % tx.message)
        # in case of  error.
        # initiate clean up and free up memory.
        ptu_cleanup(ptu)
    except Exception as e:
        print(str(e))
        # in  case of some other error, make sure cleanup happens.
        # initiate clean up and free up memory.
        # transport.close()  # close socket.
        ptu_cleanup(ptu)
    # ptu_cleanup(ptu)
    return


main()
