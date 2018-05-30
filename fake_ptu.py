
############### PTU STUFF ###############
import sys
import glob
import io
sys.path.append('./WIND-Thrift/gen-py')
sys.path.insert(0, '/home/holiestcow/thrift-0.11.0/lib/py/build/lib.linux-x86_64-3.5')

# from CVRSServices.CVRSEndpoint import Client
# from server import CVRSHandler
import CVRSServices.CVRSEndpoint
# from CVRSServices.ttypes import (StatusCode, ControlType, Session, StartRecordingControlPayload,
#                                  ControlPayloadUnion, ControlMessage, ControlMessageAck,
#                                  RecordingUpdate, DefinitionAndConfigurationUpdate)
from PTUPayload.ttypes import (UnitDefinition, UnitType, Status,
                               SystemDefinition, SystemConfiguration,
                               RecordingConfiguration, RecordingType, DataPayload)
from  CVRSServices.ttypes import (RecordingUpdate,
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
                                     NavigationSensorDefinition)
from EnvironmentalSensor.ttypes import (EnvironmentalTypes,
                                        EnvironmentalSensorDefinition)
from ContextSensor.ttypes import (ContextVideoConfiguration,
                                  ContextVideoDefinition)
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

from thrift_uuid import Thrift_UUID
from database import DatabaseOperations
import numpy as np

########## DONE IMPORTING ################

start_clock = dt.datetime.now()
uuid_dict = {}


def get_unitDefinition():
    global uuid_dict
    x = Thrift_UUID.generate_thrift_uuid()
    uuid_dict['PTU'] = UUID(
        leastSignificantBits=x[0],
        mostSignificantBits=x[1])
    # Define what I am
    unit_definition = UnitDefinition(unitId=uuid_dict['PTU'],
                                     unitName='Fake_PTU_Unit',
                                     softwareVersion='0.1',
                                     hardwareRevision='0.1',
                                     vendor='University of Tennessee - Knoxville',
                                     unitType=UnitType.Wearable)
    return unit_definition


def get_initialStatus():
    global uuid_dict
    # Define initial_status
    global uuid_dict
    x = Thrift_UUID.generate_thrift_uuid()
    uuid_dict['recordingId'] = UUID(
        leastSignificantBits=x[0],
        mostSignificantBits=x[1])
    initial_status = Status(unitId=uuid_dict['PTU'],
                            isRecording=True,
                            recordingId=uuid_dict['recordingId'],
                            hardDriveUsedPercent=0.0,  # Placeholder value
                            batteryRemainingPercent=0.0,  # Placeholder value
                            systemTime=int(time.time()))
    return initial_status


def get_gammaDefinitions():
    global uuid_dict

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

    x = Thrift_UUID.generate_thrift_uuid()
    uuid_dict['GammaDetector'] = UUID(
        leastSignificantBits=x[0],
        mostSignificantBits=x[1])

    starting_gammaSpectrumConfig = GammaListAndSpectrumConfiguration(
        componentId=uuid_dict['GammaDetector'],
        settings=sipm_pmtsettings,
        energyCalibration=energyCalibration,
        componentPositionAndOrientation=componentPositionAndOrientation,
        energyResolution=energyResolution)

    component = ComponentDefinition(
        componentId=uuid_dict['GammaDetector'],
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
            energy=energy,
            efficiency=angularEfficiencies)
        angularEfficiencyDefinitions += [angularEfficiency]

    gammaSpectrumDefinitions = [
        GammaListAndSpectrumDefinition(
            component=component,
            numberOfChannels=1024,
            physicalDimensions=physicalDimensions,
            detectorMaterial=detectorMaterial,
            startingGammaConfiguration=starting_gammaSpectrumConfig,
            angularEfficiencies=angularEfficiencyDefinitions)
    ]

    starting_gammaGrossCountConfig = GammaGrossCountConfiguration(
        componentId=uuid_dict['GammaDetector'],
        componentPositionAndOrientation=componentPositionAndOrientation)

    gammaGrossCountDefinitions = [
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

    return gammaSpectrumDefinitions, gammaGrossCountDefinitions


def get_environmentalDefinitions():
    global uuid_idct
    environmentalDefinitions = []

    x = Thrift_UUID.generate_thrift_uuid()
    uuid_dict['TemperatureSensor'] = UUID(
        leastSignificantBits=x[0],
        mostSignificantBits=x[1])

    component = ComponentDefinition(
        componentId=uuid_dict['TemperatureSensor'],
        componentName='TemperatureContextSensor',
        vendorName='University of Tennessee - Knoxville',
        serialNumber='00023224')

    componentPositionAndOrientation = GridPositionAndOrientation(
        gridPosition=Vector(x=10.0, y=10.0, z=10.0),
        rotation=Quaternion(w=0.0, x=0.0, y=0.0, z=0.0))

    sensorType = EnvironmentalTypes.Temperature
    environmentalDefinitions += [
        EnvironmentalSensorDefinition(
            component=component,
            sensorType=sensorType,
            location='somewhere',
            componentPositionAndOrientation=componentPositionAndOrientation)]
    return environmentalDefinitions


def get_navigationalDefinitions():
    global uuid_dict
    navigationalDefinitions = []

    x = Thrift_UUID.generate_thrift_uuid()
    uuid_dict['GPS'] = UUID(
        leastSignificantBits=x[0],
        mostSignificantBits=x[1])

    component = ComponentDefinition(
        componentId=uuid_dict['GPS'],
        componentName='NavigationalSensor',
        vendorName='University of Tennessee - Knoxville',
        serialNumber='00023224')
    navOutputDefinition = NavigationOutputDefinition(
        hasLatitude=True,
        hasLongitude=True,
        hasAltitude=False,
        hasX=False,
        hasY=False,
        hasZ=False,
        hasAccelerationX=False,
        hasAccelerationY=False,
        hasAccelerationZ=False,
        hasNumberOfSatellites=False,
        hasQualityOfFix=False,
        hasPitch=False,
        hasRoll=False,
        hasHeading=True,
        hasVelocityX=False,
        hasVelocityY=False,
        hasVelocityZ=False,
        hasSpeed=False)

    navigationalDefinitions += [
        NavigationSensorDefinition(
            component=component,
            navOutputDefinition=navOutputDefinition)]
    return navigationalDefinitions


def get_contextVideoDefinitions():
    global uuid_dict
    contextVideoDefinitions = None
    contextVideoConfigurations = None

    # x = Thrift_UUID.generate_thrift_uuid()
    # uuid_dict['VideoCamera'] = UUID(
    #     leastSignificantBits=x[0],
    #     mostSignificantBits=x[1])
    #
    # component = ComponentDefinition(
    #     componentId=uuid_dict['VideoCamera'],
    #     componentName='VideoCameraMyDude',
    #     vendorName='University of Tennessee - Knoxville',
    #     serialNumber='1111111')
    # componentPositionAndOrientation = GridPositionAndOrientation(
    #     gridPosition=Vector(x=-10.0, y=-10.0, z=-10.0),
    #     rotation=Quaternion(w=0.0, x=0.0, y=0.0, z=0.0))
    # videoConfiguration = ContextVideoConfiguration(
    #     componentId = uuid_dict['VideoCamera'],
    #     fileName='NANI.avi',
    #     framesPerSecond=60.0,
    #     verticalResolution=0,
    #     horizontalResolution=0,
    #     componentPositionAndOrientation=componentPositionAndOrientation,
    #     verticalFOV=verticalFOV,
    #     horizontalFOV=horizontalFOV,
    #     isRectified=True,  # No idea what this means
    #     isDeBayered=False,
    #     timeStamp=int(time.time() * 1000))
    # # NOTE: there's also "intrinsics" if isRectified is False.
    #
    # contextVideoDefinitions += [
    #     ContextVideoDefinition(
    #         component=component,
    #         videoConfiguration=videoConfiguration)]
    return contextVideoDefinitions, contextVideoConfigurations


def get_systemDefinition():
    # Defining systemDefinition

    gammaSpectrumDefinitions, gammaGrossCountDefinitions = \
        get_gammaDefinitions()
    environmentalDefinitions = get_environmentalDefinitions()
    # navigationalDefinitions = get_navigationalDefinitions()
    # contextVideoDefinition = get_contextVideoDefinitions()

    systemDefinition = SystemDefinition(
        gammaSpectrumDefinitions=gammaSpectrumDefinitions,
        gammaGrossCountDefinitions=gammaGrossCountDefinitions,
        environmentalDefinitions=environmentalDefinitions,
        timeStamp=int(time.time() * 1000),
        apiVersion='yolo')

    return systemDefinition


def construct_configurations(x, y):
    out = []
    if x is not None:
        for i in range(len(x)):
            out += [getattr(x[i], y)]
    else:
        out = None
    return out


def get_systemConfiguration(unitDefinition, systemDefinition):
    gammaSpectrumConfigurations = construct_configurations(
        systemDefinition.gammaSpectrumDefinitions,
        'startingGammaConfiguration')
    gammaListConfigurations = construct_configurations(
        systemDefinition.gammaListDefinitions,
        'startingGammaConfiguration')
    gammaGrossCountConfigurations = construct_configurations(
        systemDefinition.gammaGrossCountDefinitions,
        'startingGammaGrossCountConfiguration')
    gammaDoseConfigurations = construct_configurations(
        systemDefinition.gammaDoseDefinitions,
        'startingGammaDoseConfiguration')

    neutronListConfigurations = construct_configurations(
        systemDefinition.neutronListDefinitions,
        'startingNeutronListConfiguration')
    neutronSpectrumConfigurations = construct_configurations(
        systemDefinition.neutronSpectrumDefinitions,
        'startingNeutronSpectrumConfiguration')
    neutronGrossCountConfigurations = construct_configurations(
        systemDefinition.neutronGrossCountDefinitions,
        'startingNeutronGrossCountConfiguration')

    # NOTE: Skipping most of the video stuff for now.
    # contextVideoConfigurations = construct_configurations(
    #     systemDefinition.contextVideoDefinitions,
    #     'videoConfiguration')
    # contextPointCloudConfigurations = construct_configurations(
    #     systemDefinition.contextPointCloudDefinitions,
    #     'startingPointCloudConfiguration')
    # contextVoxelConfigurations = construct_configurations(
    #     systemDefintion.contextVoxelDefinitions,
    #     'startingVoxelCon

    # NOTE: this is also blank. Structure is not quite defined as of yet.
    # algorithmConfigurations = construct_configurations(
    #     systemDefinition.

    passiveMaterialConfigurations = systemDefinition.passiveMaterialDefinitions

    # contextStreamConfigurations = \
    #     systemDefinition.contextStreamDefinitions.configuration

    systemConfiguration = SystemConfiguration(
        unitId=unitDefinition.unitId,
        gammaSpectrumConfigurations=gammaSpectrumConfigurations,
        gammaListConfigurations=gammaListConfigurations,
        gammaGrossCountConfigurations=gammaGrossCountConfigurations,
        neutronListConfigurations=neutronListConfigurations,
        neutronSpectrumConfigurations=neutronSpectrumConfigurations,
        neutronGrossCountConfigurations=neutronGrossCountConfigurations)

    return systemConfiguration


def get_recordingUpdate():

    recordingConfig = RecordingConfiguration(
        unitId=uuid_dict['PTU'],
        recordingId=uuid_dict['recordingId'],
        campaign='Test',
        tag='yolo',
        description='Fake PTU Testing to make sure the CVRS is working as intended',
        location='Somewhere',
        fileName='PTU_local.sqlite3',
        recordingType=RecordingType.Measurement,
        recordingDuration=0,
        POSIXStartTime=int(time.time()),
        measurementNumber=1)

    recordingUpdate = RecordingUpdate(recordingConfig=recordingConfig)
    return recordingUpdate, recordingConfig


def get_gammaSpectrumData(db, lasttime):
    global uuid_dict
    # Gets the current time and just yanks the past seconds worth of data.
    # NOTE: There may be data loss if there have been payload sending failures.
    # See Issue # 1 in the github repo
    now = int(time.time() * 1000)

    lasttime = time.time() - 5

    out = []
    desired = db.get_dataSince(now - lasttime * 1000, now)
    # current = desired[-1]
    current = desired
    if current is not None:
        # Only accept the last row, see the note above.
        for row in current:
            spectrum = row[2].split(',')
            intSpectrum = [int(x) for x in spectrum]
            timestamp = int(row[0])
            # realtime = int(current[-5])
            # livetime = int(current[-10])
            realtime = 1
            livetime = 1

            integerSpectrum = Spectrum(
                spectrumInt=intSpectrum,
                format=SpectrumFormat.ARRAY,
                channelCount=len(intSpectrum),
                liveTime=livetime)

            spectrumResult = SpectrumResult(
                intSpectrum=integerSpectrum)

            out += [GammaSpectrumData(
                componentId=uuid_dict['GammaDetector'],
                timeStamp=timestamp,
                health=Health.Nominal,
                spectrum=spectrumResult,
                liveTime=livetime,
                realTime=realtime)]
    else:
        out += [GammaSpectrumData(
            componentId=uuid_dict['GammaDetector'],
            timeStamp=int(time.time() * 1000),
            health=Health.Nominal)]

    return out


def get_gammaGrossCountData(db, lasttime):
    out = []
    global uuid_dict
    # Gets the current time and just yanks the past seconds worth of data.
    # NOTE: There may be data loss if there have been payload sending failures.
    # See Issue # 1 in the github repo

    lasttime = time.time() - 5
    now = int(time.time() * 1000)
    tuples = db.get_dataSince(now - lasttime * 1000, now)
    # Only accept the last row, see the note above.
    # current = tuples.fetchall()
    # current = tuples[-1]
    current = tuples
    if current is not None:
        for row in current:
            spectrum = row[2].split(',')
            intSpectrum = [int(x) for x in spectrum]
            grossCounts = sum(intSpectrum)
            timestamp = int(row[0])
            realtime = 1
            livetime = 1

            out += [GammaGrossCountData(
                componentId=uuid_dict['GammaDetector'],
                timeStamp=timestamp,
                health=Health.Nominal,
                counts=grossCounts,
                liveTime=livetime,
                realTime=realtime)]
    else:
        out += [GammaGrossCountData(
            componentId=uuid_dict['GammaDetector'],
            timeStamp=int(time.time() * 1000),
            health=Health.Nominal)]
    return out


def main():
    global uuid_dict
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

    db = DatabaseOperations('./PTU_local.sqlite3')
    # db.initialize_structure(numdetectors=1)
    counter = 0
    lasttime = time.time()
    while True:
        time.sleep(1.0)  # Sleep for one second
        a = time.time()
        # 1) Look for  file changes and dump to the database
        # 2) report the status (should be the same message)
        status = get_initialStatus()
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
        print('Report Status {}s'.format(d-c))

        c = time.time()
        # 3) pushData
        gammaSpectrumData = get_gammaSpectrumData(db, lasttime)
        # gammaListData  = get_gammaListData(db)
        gammaListData = []
        gammaGrossCountData = get_gammaGrossCountData(db, lasttime)
        # gammaDoseData = get_gammaDoseData(db)
        gammaDoseData = []
        # neutronListData = get_neutronListData(db)
        neutronListData = []
        # neutronSpectrumData = get_neutronSpectrumData(db)
        neutronSpectrumData = []
        # neutronGrossCountData = get_neutronGrossCountData(db)
        neutronGrossCountData = []
        # environmentalData = get_environmentalData(db)
        environmentalData = []
        # navigationData = get_navigationData(db)
        navigationData = []
        # videoData = get_videoData(db)
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
            unitId=uuid_dict['PTU'],
            timeStamp=int(time.time() * 1000),
            systemHealth=Health.Nominal,
            isEOF=False,
            # recordingConfig=recordingConfiguration,
            gammaSpectrumData=gammaSpectrumData,
            # gammaListData=gammaListData,
            gammaGrossCountData=gammaGrossCountData)
            # gammaDoseData=gammaDoseData,
            # neutronListData=neutronListData,
            # neutronSpectrumData=neutronSpectrumData,
            # neutronGrossCountData=neutronGrossCountData,
            # environmentalData=environmentalData,
            # navigationData=navigationData,
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
        isGood = client.pushData(
            sessionId=session.sessionId,
            datum=dataPayload,
            definitionAndConfigurationUpdate=definitionAndConfigurationUpdate)
        d = time.time()
        print('Payload delivery {}s'.format(d-c))
        while not isGood:
            print('delivery failure')
            # not sure if sleep is good here. or continuous trying
            isGood = client.pushData(
                sessionId=session.sessionId,
                datum=dataPayload,
                definitionAndConfigurationUpdate=definitionAndConfigurationUpdate)

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

if __name__ == '__main__':
    try:
        main()
    except Thrift.TException as tx:
        print('%s' % tx.message)
