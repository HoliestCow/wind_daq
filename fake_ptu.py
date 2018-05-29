
############### PTU STUFF ###############
import sys
import glob
sys.path.append('./WIND-Thrift/gen-py')
sys.path.insert(0, '/home/holiestcow/thrift-0.11.0/lib/py/build/lib.linux-x86_64-3.5')

# from CVRSServices.CVRSEndpoint import Client
# from server import CVRSHandler
import CVRSServices.CVRSEndpoint
# from CVRSServices.ttypes import (StatusCode, ControlType, Session, StartRecordingControlPayload,
#                                  ControlPayloadUnion, ControlMessage, ControlMessageAck,
#                                  RecordingUpdate, DefinitionAndConfigurationUpdate)
from PTUPayload.ttypes import UnitDefinition
# from server import
from Exceptions.ttypes import InvalidSession

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.server import TServer
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
    uuid_dict['PTU'] = Thrift_UUID.generate_thrift_uuid()[-1]
    # Define what I am
    unit_definition = UnitDefinition(unitId=uuid_dict['PTU'],
                                     unitName='Fake_PTU_Unit',
                                     softwareVersion='0.1',
                                     hardwareRevision='0.1',
                                     vendor='University of Tennessee - Knoxville',
                                     unitType='Wearable')
    return unit_definition


def get_initialStatus():
    global uuid_dict
    # Define initial_status
    uuid_dict['recordingId'] = Thrift_UUID.generate_thrift_uuid()[-1]
    initial_status = Status(unitId=uuid_dict['PTU'],
                            isRecording=True,
                            recordingId=uuid_dict['recordingId'],
                            hardDriveUsedPercent=0.0,  # Placeholder value
                            batteryRemainingPercent=0.0,  # Placeholder value
                            systemTime=int(time.time()))
    return


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

    uuid_dict['GammaDetector'] = Thrift_UUID.generate_thrift_uuid()[-1]

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
    detectorMaterial = DetectorMaterial(NaI)

    angularEfficiencyDefinitions = []
    for i in range(0, 1024):  # by energy
        energy = (i + 1) * dE
        angularEfficiences = []
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
            startingGammaConfiguration=startingGammaConfiguration,
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
            startingGammaGrossCountConfiguration=gammaGrossCountConfig,
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
    global uuid_thrift
    environmentalDefinitions = []

    uuid_thrift['TemperatureSensor'] = Thrift_UUID.generate_thrift_uuid()[-1]

    component = ComponentDefinition(
        componentId=uuid_thrift['TemperatureSensor'],
        componentName='TemperatureContextSensor',
        vendorName='University of Tennessee - Knoxville',
        serialNumber='00023224')

    componentPositionAndOrientation = GridPositionAndOrientation(
        gridPosition=Vector(x=10.0, y=10.0, z=10.0),
        rotation=Quaternion(w=0.0, x=0.0, y=0.0, z=0.0))

    sensorType = EnvironmentalTypes(Temperature)
    environmentalDefinitions += [
        EnvironmentalSensorDefinition(
            component=component,
            sensorType=sensorType,
            location='somewhere',
            componentPositionAndOrientation=componentPositionAndOrientation)]
    return environmentalDefinitions


def get_navigationalDefinitions():
    global uuid_thrift
    navigationalDefinitions = []

    uuid_thrift['GPS'] = Thrift_UUID.generate_thrift_uuid()[-1]

    component = ComponentDefinition(
        componentId=uuid_thrift['GPS'],
        componentName='NavigationalSensor',
        vendorName='University of Tennessee - Knoxville',
        serialNumber='00023224')
    navOutputDefinition = navOutputDefinition(
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
    global uuid_thrift
    contextVideoDefinitions = []

    uuid_thrift['VideoCamera'] = Thrift_UUID.generate_thrift_uuid()[-1]

    component = ComponentDefinition(
        componentId=uuid_thrift['VideoCamera'],
        componentName='VideoCameraMyDude',
        vendorName='University of Tennessee - Knoxville',
        serialNumber='1111111')
    componentPositionAndOrientation = GridPositionAndOrientation(
        gridPosition=Vector(x=-10.0, y=-10.0, z=-10.0),
        rotation=Quaternion(w=0.0, x=0.0, y=0.0, z=0.0))
    videoConfiguration = ContextVideoConfiguration(
        componentId = uuid_thrift['VideoCamera'],
        fileName='NANI.avi',
        framesPerSecond=60.0,
        verticalResolution=0,
        horizontalResolution=0,
        componentPositionAndOrientation=componentPositionAndOrientation,
        verticalFOV=verticalFOV,
        horizontalFOV=horizontalFOV,
        isRectified=True,  # No idea what this means
        isDeBayered=False,
        timeStamp=int(time.time() * 1000))
    # NOTE: there's also "intrinsics" if isRectified is False.

    contextVideoDefinitions += [
        ContextVideoDefinition(
            component=component,
            videoConfiguration=videoConfiguration)]
    return contextVideoDefinitions


def get_systemDefinition():
    # Defining systemDefinition

    gammaSpectrumDefinitions, gammaGrossCountDefinitions = \
        get_gammaDefinitions()
    environmentalDefinitions = get_environmentalDefinitions()
    navigationalDefinitions = get_navigationalDefinitions()
    contextVideoDefinition = get_contextVideoDefinitions()

    systemDefinition = SystemDefinition(
        gammaSpectrumDefinitions=gammaSpectrumDefinitions,
        # gammaListDefinitions=  # NOTE: I haven't set this up yet.
        gammaGrossCountDefinitions=gammaGrossCountDefinitions,
        gammaDoseDefinitions=[],
        neutronListDefinitions=[],
        neutronSpectrumDefinitions=[],
        neutronGrossCountDefinitions=[],
        environmentalDefinitions=environmentalDefinitions,
        navigationSensorDefinitions=navigationalDefinitions,
        contextVideoDefinitions=contextVideoDefinition,
        contextPointCloudDefinitions=[],
        contextVoxelDefinitions=[],
        contextMeshDefinitions=[],
        algorithmDefinitions=[],
        apiVersion=PROTOCOL_VERSION,
        passiveMaterialDefinitions=[],
        timeStamp=int(time.time() * 1000),
        contextStreamDefinitions=[])

    return systemDefinition


def construct_configurations(x, y):
    out = []
    for i in range(len(x)):
        out += [getattr(x[i], y)]
    return out


def get_systemConfiguration(unitDefinition, systemDefinition):
    gammaSpectrumConfigurations = construct_configurations(
        systemDefinition.gammaSpectrumDefinitions,
        'startingGammaConfiguration')
    gammaListConfigurations = construct_configurations(
        systemDefinition.gammaListDefinitions,
        'startingGammaConfiguration')
    gammaGrossCountConfigurations = constrcut_configurations(
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
    contextVideoConfigurations = construct_configurations(
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

    passiveMaterialConfigurations = systemDefinition.passiveMaterialDefinitions

    contextStreamConfigurations = \
        systemDefinition.contextStreamDefinitions.configuration

    systemConfiguration = SystemConfiguration(
        unitId=unitDefinition.unitId,
        gammaSpectrumConfigurations=gammaSpectrumConfigurations,
        gammaListConfigurations=gammaListConfigurations,
        gammaGrossCountConfigurations=gammaGrossCountConfigurations,
        neutronListConfigurations=neutronListConfigurations,
        neutronSpectrumConfigurations=neutronSpectrumConfigurations,
        neutronGrossCountConfigurations=neturonGrossCountConfigurations,
        contextVideoConfigurations=contextVideoConfigurations,
        apiVersion=PROTOCOL_VERSION,
        passiveMaterialDefinitions=passiveMaterialConfigurations,
        contextStreamConfigurations=contextStreamConfigurations)

    return systemConfiguration


def get_recordingUpdate():
    global uuid_dict
    uuid_dict['recordingID'] = Thrift_UUID.generate_thrift_uuid()[-1]

    recordingConfig = RecordingConfiguration(
        unitId=uuid_dict['PTU'],
        recordingId=uuid_dict['recordingID'],
        campaign='Test',
        tag='yolo',
        description='Fake PTU Testing to make sure the CVRS is working as intended',
        location='Somewhere',
        fileName='PTU_local.sqlite3',
        recordingType=RecordingType(Measurement),
        recordingDuration=0,
        POSIXStartTime=int(time.time()),
        measurementNumber=1)

    recordingUpdate = RecordingUpdate(recordingConfig=recordingConfig)
    return recordingUpdate, recordingConfig


def dumptodb(db, t, datum):
    print('bathroom')
    hacked_spectrum = datum.tolist()
    hacked_spectrum = [str(x) for x in hacked_spectrum]
    hacked_spectrum = ','.join(hacked_spectrum)
    #  (I'm going off of the train dataset)
    desired_outputs = (t,  # this is timestamp
                       0,  # PositionID
                       hacked_spectrum,
                       np.sum(datum),  # cps
                       1,  # isAlive
                       0,  # Energy Cubic a  Not sure where to get this data from. Fuck it.
                       0,  # Energy Cubic b
                       0,  # Energy Cubic c
                       'Spectrum',  # Data type
                       t,
                       0,  # Upper ROI counts
                       t,
                       0,  # Lower ROI counts
                       0,  # Fine Gain
                       0.1,  # Response Time
                       600,  # High voltage
                       t,
                       -2,  # TickDelta
                       0,  #  TickNumber
                       0,  # Coarse gain
                       0)  # Energy cubic d
    db.fake_stack_datum(desired_outputs)
    return


def get_gammaSpectrumData(db):
    global uuid_dict
    # Gets the current time and just yanks the past seconds worth of data.
    # NOTE: There may be data loss if there have been payload sending failures.
    # See Issue # 1 in the github repo
    now = int(time.time() * 1000)

    out = []

    tuples = db.get_dataSince(now - 1000, now)
    # Only accept the last row, see the note above.
    current = tuples[-1]
    spectrum = current[2].split(',')
    intSpectrum = [int(x) for x in spectrum]
    doubleSpectrum = [float(x) for x in spectrum]
    timestamp = int(current[9])
    realtime = int(current[-5])
    livetime = int(current[-10])

    spectrumResult = SpectrumResult(
        intSpectrum=intSpectrum,
        doubleSpectrum=doubleSpectrum)

    out += [GammaSpectrumData(
        componentId=uuid_dict['GammaDetector'],
        timeStamp=timestamp,
        systemHealth=Health(Nominal),
        isEOF=False,
        spectrum=spectrumResult,
        liveTime=livetime,
        realTime=realtime)]
    return out


def get_gammaGrossCountData(db):
    out = []
    global uuid_dict
    # Gets the current time and just yanks the past seconds worth of data.
    # NOTE: There may be data loss if there have been payload sending failures.
    # See Issue # 1 in the github repo
    now = int(time.time() * 1000)
    tuples = db.get_dataSince(now - 1000, now)
    # Only accept the last row, see the note above.
    current = tuples[-1]
    spectrum = current[2].split(',')
    intSpectrum = [int(x) for x in spectrum]
    grossCounts = sum(intSpectrum)
    timestamp = int(current[9])
    realtime = int(current[-5])
    livetime = int(current[-10])

    out += [GammaGrossCountData(
        componentId=uuid_dict['GammaDetector'],
        timeStamp=timestamp,
        systemHealth=Health(Nominal),
        counts=grossCounts,
        liveTime=livetime,
        realTime=realtime)]
    return out


def main():
    global uuid_dict
    # Make socket
    transport = TSocket.TSocket('0.0.0.0', 8050)

    # Buffering is critical. Raw sockets are very slow
    transport = TTransport.TBufferedTransport(transport)

    # Wrap in a protocol
    protocol = TBinaryProtocol.TBinaryProtocol(transport)

    # Create a client to use the protocol encoder
    # client = CVRSHandler.Client(protocol)
    # client = Client(protocol)
    client = CVRSServices.CVRSEndpoint.Client(protocol)
    client.ping()

    # Connect!
    transport.open()

    unit_definition = get_unitDefinition()
    # Initiate handshake
    session = client.registerPtu(unitDefinition=unit_definition)
    time.sleep(5)  # this method returns a session class from CVRSServices

    status = get_initialStatus()
    systemDefinition = get_systemDefinition()
    systemConfiguration = get_systemConfiguration(systemDefinition)
    recordingUpdate, recordingConfiguration = get_recordingUpdate()

    ptu_message = client.define(session.sessionId, status,
                                systemDefinition, systemConfiguration,
                                recordingUpdate)

    targetFile = './data/raw_stream_data.dat'
    isInitialize = True
    db = DatabaseOperations('./PTU_local.sqlite3')
    db.initialize_structure(numdetectors=1)
    counter = 0
    with io.open(targetFile, 'r', buffering=1) as f:
        f.seek(0, 2) # Go to the end of the file
        t = int(time.time())
        juice = []
        while True:
            # time.sleep(1)  # Sleep for one second
            # NOTE: The "1Hz" clock is governed by measurement since I don't have a formal server client architecture setup between the packaging layer and the measurement layer. Time per report is a little  over a second since an interaction has to occur before it sends the payload
            # 1) Look for  file changes and dump to the database
            line = f.readline().strip()
            if not line:
                time.sleep(0.01)  # Sleep for 10 milliseconds
                continue
            if isInitialize:
                isInitialize = False
                words = line.split()
                prev_time = float(words[0])
            words = line.split()
            x = float(words[0])
            counter += x
            charge = float(words[1])
            if counter >= 1:
                counts, bin_edges = np.histogram(np.array(juice), bins=1024, range=(0, 28000))
                # bin_edges = bin_edges[1:]
                dumptodb(db, t * 1000, counts)
                juice = []
                t += 1
                counter = 0

                # 2) report the status (should be the same message)
                status = get_initialStatus()
                definitionAndConfigurationUpdate = DefinitionAndConfigurationUpdate(
                    systemDefinition=systemDefinition,
                    systemConfiguration=systemConfiguration)

                isGood = client.reportStatus(
                    sessionId=session.sessionId,
                    status=status,
                    definitionAndConfigurationUpdate=definitionAndConfigurationUpdate)

                if not isGood:
                    break
                # 3) pushData
                gammaSpectrumData = get_gammaSpectrumData(db)
                # gammaListData  = get_gammaListData(db)
                gammaListData = []
                gammaGrossData = get_gammaGrossCountData(db)
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
                configuration = systemConfiguration

                dataPayload = DataPayload(
                    unitId=uuid_dict['PTU'],
                    timeStamp=int(time.time() * 1000),
                    systemHealth=Health(Nominal),
                    isEOF=False,
                    recordingConfig=recordingConfiguration,
                    gammaSpectrumData=gammaSpectrumData,
                    gammaListData=gammaListData,
                    gammaGrossCountData=gammaGrossCountData,
                    gammaDoseData=gammaDoseData,
                    neutronListData=neutronListData,
                    neutronSpectrumData=neutronSpectrumData,
                    neutronGrossCountData=neutronGrossCountData,
                    environmentalData=environmentalData,
                    navigationData=navigationData,
                    videoData=videoData,
                    pointCloudData=pointCloudData,
                    voxelData=voxelData,
                    meshData=meshData,
                    messages=messages,
                    waypoints=waypoints,
                    boundingBoxes=boundingoboxes,
                    markers=markers,
                    algorithmData=algorithmData,
                    streamIndexData=streamIndexData,
                    configuration=configuration)

                isGood = client.pushData(
                    sessionId=session.sessionId,
                    datum=dataPayload,
                    definitionAndConfigurationUpdate=definitionAndConfigurationUpdate)

                while not isGood:
                    # not sure if sleep is good here. or continuous trying
                    time.sleep(0.1)
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
            juice += [float(charge)]



    # NOTE: Frankly I should never get to the close, since I'll be infinitely looping
    transport.close()
    return

if __name__ == '__main__':
    try:
        main()
    except Thrift.TException as tx:
        print('%s' % tx.message)
