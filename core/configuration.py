
from wind_daq.thrift.pyout.PTUPayload.ttypes import (SystemConfiguration,
                                                     SystemDefinition,
                                                     SIPMSettings,
                                                     SIPM_PMTSettings,
                                                     GammaListAndSpectrumDefinition)
from wind_daq.thrift.pyout.GammaSensor import (GammaListAndSpectrumConfiguration,
                                               GammaGrossCountConfiguration,
                                               GammaDoseConfiguration,
                                               GammaGrossCountDefinition,
                                               GammaDoseDefinition)
from wind_daq.thrift.pyout.DetectorCharacteristics import (EnergyCalibration,
                                                           RectangularDimensions,
                                                           Dimensions)
from wind_daq.thrift.pyout.ComponentLocation import GridPositionAndOrientation
import thrift_uuid
import numpy as np

# DEFINE UUID
channel0_uuid = thrift_uuid.generate_thrift_uuid_with_name('NaIBar_1')
channel1_uuid = thrift_uuid.generate_thrift_uuid_with_name('NaIBar_2')

###########################################################
# ENERGY CALIBRATION. CONSTANT FOR ALL DETECTORS FOR NOW ##
###########################################################
# What are the units of energy???
# channel_0_energycalibration = EnergyCalibration(channel=2**15, energy=4.3)
# assume linear energy calibration, 15 bit adc, 4.3 MeV max
# assume that the energy calibration is the same across all detectors
max_energy = 4.3 * 1000  # assuming it's in keV
delta_energy = 4.3 * 1000 / 2**15  # assuming it's in keV
channel_index = np.arange(0, 2**15)
channel_index2energy = np.linspace(delta_energy, max_energy, delta_energy)
energyCalibration = []
for i in range(0, len(channel_index)):
    energyCalibration += [EnergyCalibration(channel=channel_index[i],
                                            energy=channel_index2energy[i])]

# TODO: Is this class dependent on channel or device?


# DEFINING CONFIGURATIONS #

##########################################
# NaIBar_1 ###############################
##########################################
channel_0_gamma_sipm = SIPMSettings(highvoltage=9.)
channel_0_settings = SIPM_PMTSettings(sipmSettings=channel_0_gamma_sipm, pmtSettings=None)

channel_0_location = GridPositionAndOrientation(
    gridPositionX=None,
    gridPositionY=None,
    gridPositionZ=None,
    rotationQuaternionW=None,
    rotationQuaternionX=None,
    rotationQuaternionY=None,
    rotationQuaternionZ=None
)

channel_0_gamma_spec_config = GammaListAndSpectrumConfiguration(
    componentId=channel0_uuid,  # TODO: different component id??
    settings=channel_0_settings,
    energyCalibration=energyCalibration,
    componentPositionAndOrientation=channel_0_location)
channel_0_gamma_list_config = GammaListAndSpectrumConfiguration(
    componentId=channel0_uuid,  # TODO: different component id??
    settings=channel_0_settings,
    energyCalibration=energyCalibration,
    componentPositionAndOrientation=channel_0_location)
channel_0_gamma_grosscount_config = GammaGrossCountConfiguration(
    componentId=channel0_uuid,
    componentPositionAndOrientation=channel_0_location)
channel_0_gamma_dose_config = GammaDoseConfiguration(
    componentId=channel0_uuid,
    componentPositionAndOrientation=channel_0_location)

##########################################
# NaIBar_2 ###############################
##########################################
channel_1_gamma_sipm = SIPMSettings(highvoltage=9.)
channel_1_settings = SIPM_PMTSettings(sipmSettings=channel_1_gamma_sipm, pmtSettings=None)

channel_1_location = GridPositionAndOrientation(
    gridPositionX=None,
    gridPositionY=None,
    gridPositionZ=None,
    rotationQuaternionW=None,
    rotationQuaternionX=None,
    rotationQuaternionY=None,
    rotationQuaternionZ=None
)

channel_1_gamma_spec_config = GammaListAndSpectrumConfiguration(
    componentId=channel1_uuid,  # TODO: different component id??
    settings=channel_1_settings,
    energyCalibration=energyCalibration,
    componentPositionAndOrientation=channel_1_location)
channel_1_gamma_list_config = GammaListAndSpectrumConfiguration(
    componentId=channel1_uuid,  # TODO: different component id??
    settings=channel_1_settings,
    energyCalibration=energyCalibration,
    componentPositionAndOrientation=channel_1_location)
channel_1_gamma_grosscount_config = GammaGrossCountConfiguration(
    componentId=channel1_uuid,
    componentPositionAndOrientation=channel_1_location)
channel_1_gamma_dose_config = GammaDoseConfiguration(
    componentId=channel1_uuid,
    componentPositionAndOrientation=channel_1_location)

# TODO: get CLYC working. Leave commented
# CLYC
# Channel_2_spec_config = GammaListAndSpectrumConfiguration(
#                 componentId=None,
#                 settings=None,
#                 energyCalibration=energyCalibration,
#                 componentPositionAndOrientation=None)

gammaSpecConfigs = [channel_0_gamma_spec_config, channel_1_gamma_spec_config]
gammaListConfigs = [channel_0_gamma_list_config, channel_1_gamma_list_config]
gammaGrossConfigs = [channel_0_gamma_grosscount_config, channel_1_gamma_grosscount_config]
gammaDoseConfigs = [channel_0_gamma_dose_config, channel_1_gamma_dose_config]

system_uuid = thrift_uuid.generate_thrift_uuid_with_name('UTKWIND_Array')

GAMMASPECTRUM_CONFIGURATION = gammaSpecConfigs
GAMMALIST_CONFIGURATION = gammaListConfigs

SYSTEM_CONFIGURATION = SystemConfiguration(
    unitId=system_uuid,
    gammaSpectrumConfigurations=GAMMASPECTRUM_CONFIGURATION,
    gammaListConfigurations=GAMMALIST_CONFIGURATION,
    gammaGrossCountConfigurations=gammaGrossConfigs,
    gammaDoseConfigurations=gammaDoseConfigs)
# DEFINING DEFINITIONS #

channel_0_name = 'NaIBar_1'
channel_1_name = 'NaIBar_2'
vendor_name = 'UTKRadIDEAS'  # This may be Scionix
# 54x54x152mm
RPP = RectangularDimensions(length=54.0/10.0,
                            width=54.0/10.0,
                            height=152.0/10.0)
naibar_dimensions = Dimensions(cylindricalDimensions=None,
                               sphericalDimensions=None,
                               rectangularDimensions=RPP)
naibar_material = 'NaI'  # This may need to be a number
startingGammaConfig = channel_1_gamma_spec_config  # TODO: Check this
angularEfficiencies = None  # ???
numberofchannels = 2**15

channel_0_gamma_spectrum_definition = GammaListAndSpectrumDefinition(
    componentId=channel0_uuid,
    componentName=channel_0_name,
    vendorName=vendor_name,
    serialNumber=None,
    numberOfChannels=numberofchannels,
    physicalDimensions=naibar_dimensions,
    detectorMaterial=naibar_material,
    startingGammaConfiguration=channel_0_gamma_spec_config,
    angularEfficiencies=angularEfficiencies)
channel_1_gamma_spectrum_definition = GammaListAndSpectrumDefinition(
    componentId=channel1_uuid,
    componentName=channel_1_name,
    vendorName=vendor_name,
    serialNumber=None,
    numberOfChannels=numberofchannels,
    physicalDimensions=naibar_dimensions,
    detectorMaterial=naibar_material,
    startingGammaConfiguration=channel_1_gamma_spec_config,
    angularEfficiencies=angularEfficiencies)

channel_0_gamma_list_definition = GammaListAndSpectrumDefinition(
    componentId=channel0_uuid,
    componentName=channel_0_name,
    vendorName=vendor_name,
    serialNumber=None,
    numberOfChannels=numberofchannels,
    physicalDimensions=naibar_dimensions,
    detectorMaterial=naibar_material,
    startingGammaConfiguration=channel_0_gamma_list_config,
    angularEfficiencies=angularEfficiencies)
channel_1_gamma_list_definition = GammaListAndSpectrumDefinition(
    componentId=channel1_uuid,
    componentName=channel_1_name,
    vendorName=vendor_name,
    serialNumber=None,
    numberOfChannels=numberofchannels,
    physicalDimensions=naibar_dimensions,
    detectorMaterial=naibar_material,
    startingGammaConfiguration=channel_1_gamma_list_config,  # TODO: This is probably fucked
    angularEfficiencies=angularEfficiencies)

channel_0_gamma_grosscount_definition = GammaGrossCountDefinition(
    componentId=channel0_uuid,
    componentName=channel_0_name,
    vendorName=vendor_name,
    serialNumber=None,
    physicalDimensions=naibar_dimensions,
    detectorMaterial=naibar_material,
    startingGammaGrossCountConfiguration=channel_0_gamma_grosscount_config,
    angularEfficiencies=angularEfficiencies)
channel_1_gamma_grosscount_definition = GammaGrossCountDefinition(
    componentId=channel1_uuid,
    componentName=channel_1_name,
    vendorName=vendor_name,
    serialNumber=None,
    physicalDimensions=naibar_dimensions,
    detectorMaterial=naibar_material,
    startingGammaGrossCountConfiguration=channel_1_gamma_grosscount_config,
    angularEfficiencies=angularEfficiencies)

# TODO: Are the angular efficiencies for each of these GAMMA definitions the same???
# TODO: Units of angular efficiency???

channel_0_gamma_dose_definition = GammaDoseDefinition(
    componentId=channel0_uuid,
    componentName=channel_0_name,
    vendorName=vendor_name,
    serialNumber=None,
    physicalDimensions=naibar_dimensions,
    detectorMaterial=naibar_material,
    startingGammaDoseConfiguration=channel_0_gamma_dose_config,
    angularEfficiencies=angularEfficiencies)
channel_1_gamma_dose_definition = GammaDoseDefinition(
    componentId=channel1_uuid,
    componentName=channel_1_name,
    vendorName=vendor_name,
    serialNumber=None,
    physicalDimensions=naibar_dimensions,
    detectorMaterial=naibar_material,
    startingGammaDoseConfiguration=channel_1_gamma_dose_config,
    angularEfficiencies=angularEfficiencies)

system_gamma_spectrum_definitions = [channel_0_gamma_spectrum_definition,
                                     channel_1_gamma_spectrum_definition]
system_gamma_list_definintions = [channel_0_gamma_list_definition,
                                  channel_1_gamma_list_definition]
system_gamma_grosscount_definitions = [channel_0_gamma_grosscount_definition,
                                       channel_1_gamma_grosscount_definition]
system_gamma_dose_definitions = [channel_0_gamma_dose_definition,
                                 channel_1_gamma_dose_definition]

SYSTEM_DEFINITION = SystemDefinition(
    gammaSpectrumDefinitions=system_gamma_spectrum_definitions,
    gammaListDefinitions=system_gamma_list_definintions,
    gammaGrossCountDefinitions=system_gamma_grosscount_definitions,
    gammaDoseDefinitions=system_gamma_dose_definitions)























