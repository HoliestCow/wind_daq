#
# Autogenerated by Thrift Compiler (0.11.0)
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#
#  options string: py
#

from thrift.Thrift import TType, TMessageType, TFrozenDict, TException, TApplicationException
from thrift.protocol.TProtocol import TProtocolException
from thrift.TRecursive import fix_spec

import sys
import Identification.ttypes
import Spectrum.ttypes

from thrift.transport import TTransport
all_structs = []


class DetectionConfiguration(object):
    """
    Configuration

    Attributes:
     - nuclidesSensitivity: List of nuclides this algorithm can detect
     - backgroundWindowSize: Time in milliseconds of background background window
     - foregroundWindowSize: Time in milliseconds of foreground window
     - lowerAlarmThreshold: Scalar
     - upperAlarmThreshold: Scalar
     - lowerROIThreshold: KeV
     - upperROIThreshold: KeV
    """


    def __init__(self, nuclidesSensitivity=None, backgroundWindowSize=None, foregroundWindowSize=None, lowerAlarmThreshold=None, upperAlarmThreshold=None, lowerROIThreshold=None, upperROIThreshold=None,):
        self.nuclidesSensitivity = nuclidesSensitivity
        self.backgroundWindowSize = backgroundWindowSize
        self.foregroundWindowSize = foregroundWindowSize
        self.lowerAlarmThreshold = lowerAlarmThreshold
        self.upperAlarmThreshold = upperAlarmThreshold
        self.lowerROIThreshold = lowerROIThreshold
        self.upperROIThreshold = upperROIThreshold

    def read(self, iprot):
        if iprot._fast_decode is not None and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None:
            iprot._fast_decode(self, iprot, [self.__class__, self.thrift_spec])
            return
        iprot.readStructBegin()
        while True:
            (fname, ftype, fid) = iprot.readFieldBegin()
            if ftype == TType.STOP:
                break
            if fid == 1:
                if ftype == TType.LIST:
                    self.nuclidesSensitivity = []
                    (_etype3, _size0) = iprot.readListBegin()
                    for _i4 in range(_size0):
                        _elem5 = iprot.readI32()
                        self.nuclidesSensitivity.append(_elem5)
                    iprot.readListEnd()
                else:
                    iprot.skip(ftype)
            elif fid == 2:
                if ftype == TType.I64:
                    self.backgroundWindowSize = iprot.readI64()
                else:
                    iprot.skip(ftype)
            elif fid == 3:
                if ftype == TType.I64:
                    self.foregroundWindowSize = iprot.readI64()
                else:
                    iprot.skip(ftype)
            elif fid == 4:
                if ftype == TType.DOUBLE:
                    self.lowerAlarmThreshold = iprot.readDouble()
                else:
                    iprot.skip(ftype)
            elif fid == 5:
                if ftype == TType.DOUBLE:
                    self.upperAlarmThreshold = iprot.readDouble()
                else:
                    iprot.skip(ftype)
            elif fid == 6:
                if ftype == TType.DOUBLE:
                    self.lowerROIThreshold = iprot.readDouble()
                else:
                    iprot.skip(ftype)
            elif fid == 7:
                if ftype == TType.DOUBLE:
                    self.upperROIThreshold = iprot.readDouble()
                else:
                    iprot.skip(ftype)
            else:
                iprot.skip(ftype)
            iprot.readFieldEnd()
        iprot.readStructEnd()

    def write(self, oprot):
        if oprot._fast_encode is not None and self.thrift_spec is not None:
            oprot.trans.write(oprot._fast_encode(self, [self.__class__, self.thrift_spec]))
            return
        oprot.writeStructBegin('DetectionConfiguration')
        if self.nuclidesSensitivity is not None:
            oprot.writeFieldBegin('nuclidesSensitivity', TType.LIST, 1)
            oprot.writeListBegin(TType.I32, len(self.nuclidesSensitivity))
            for iter6 in self.nuclidesSensitivity:
                oprot.writeI32(iter6)
            oprot.writeListEnd()
            oprot.writeFieldEnd()
        if self.backgroundWindowSize is not None:
            oprot.writeFieldBegin('backgroundWindowSize', TType.I64, 2)
            oprot.writeI64(self.backgroundWindowSize)
            oprot.writeFieldEnd()
        if self.foregroundWindowSize is not None:
            oprot.writeFieldBegin('foregroundWindowSize', TType.I64, 3)
            oprot.writeI64(self.foregroundWindowSize)
            oprot.writeFieldEnd()
        if self.lowerAlarmThreshold is not None:
            oprot.writeFieldBegin('lowerAlarmThreshold', TType.DOUBLE, 4)
            oprot.writeDouble(self.lowerAlarmThreshold)
            oprot.writeFieldEnd()
        if self.upperAlarmThreshold is not None:
            oprot.writeFieldBegin('upperAlarmThreshold', TType.DOUBLE, 5)
            oprot.writeDouble(self.upperAlarmThreshold)
            oprot.writeFieldEnd()
        if self.lowerROIThreshold is not None:
            oprot.writeFieldBegin('lowerROIThreshold', TType.DOUBLE, 6)
            oprot.writeDouble(self.lowerROIThreshold)
            oprot.writeFieldEnd()
        if self.upperROIThreshold is not None:
            oprot.writeFieldBegin('upperROIThreshold', TType.DOUBLE, 7)
            oprot.writeDouble(self.upperROIThreshold)
            oprot.writeFieldEnd()
        oprot.writeFieldStop()
        oprot.writeStructEnd()

    def validate(self):
        return

    def __repr__(self):
        L = ['%s=%r' % (key, value)
             for key, value in self.__dict__.items()]
        return '%s(%s)' % (self.__class__.__name__, ', '.join(L))

    def __eq__(self, other):
        return isinstance(other, self.__class__) and self.__dict__ == other.__dict__

    def __ne__(self, other):
        return not (self == other)


class DetectionDefinition(object):
    """
    Definition

    Attributes:
     - startingDetectionConfiguration
    """


    def __init__(self, startingDetectionConfiguration=None,):
        self.startingDetectionConfiguration = startingDetectionConfiguration

    def read(self, iprot):
        if iprot._fast_decode is not None and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None:
            iprot._fast_decode(self, iprot, [self.__class__, self.thrift_spec])
            return
        iprot.readStructBegin()
        while True:
            (fname, ftype, fid) = iprot.readFieldBegin()
            if ftype == TType.STOP:
                break
            if fid == 1:
                if ftype == TType.STRUCT:
                    self.startingDetectionConfiguration = DetectionConfiguration()
                    self.startingDetectionConfiguration.read(iprot)
                else:
                    iprot.skip(ftype)
            else:
                iprot.skip(ftype)
            iprot.readFieldEnd()
        iprot.readStructEnd()

    def write(self, oprot):
        if oprot._fast_encode is not None and self.thrift_spec is not None:
            oprot.trans.write(oprot._fast_encode(self, [self.__class__, self.thrift_spec]))
            return
        oprot.writeStructBegin('DetectionDefinition')
        if self.startingDetectionConfiguration is not None:
            oprot.writeFieldBegin('startingDetectionConfiguration', TType.STRUCT, 1)
            self.startingDetectionConfiguration.write(oprot)
            oprot.writeFieldEnd()
        oprot.writeFieldStop()
        oprot.writeStructEnd()

    def validate(self):
        return

    def __repr__(self):
        L = ['%s=%r' % (key, value)
             for key, value in self.__dict__.items()]
        return '%s(%s)' % (self.__class__.__name__, ', '.join(L))

    def __eq__(self, other):
        return isinstance(other, self.__class__) and self.__dict__ == other.__dict__

    def __ne__(self, other):
        return not (self == other)


class DetectionResult(object):
    """
    Result

    Attributes:
     - decisionMetric
     - resultantSpectrum
     - inAlarm
     - gammaDose: This is the total gamma dose estimated at center.  (origin of the detector system)

    This is not the sum of the dose on each detector.
     - neutronDose: This is the total neutron dose estimated at center.  (origin of the detector system)

    This is not the sum of the dose on each detector.  May be based on the neutron and gamma data.
    """


    def __init__(self, decisionMetric=None, resultantSpectrum=None, inAlarm=None, gammaDose=None, neutronDose=None,):
        self.decisionMetric = decisionMetric
        self.resultantSpectrum = resultantSpectrum
        self.inAlarm = inAlarm
        self.gammaDose = gammaDose
        self.neutronDose = neutronDose

    def read(self, iprot):
        if iprot._fast_decode is not None and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None:
            iprot._fast_decode(self, iprot, [self.__class__, self.thrift_spec])
            return
        iprot.readStructBegin()
        while True:
            (fname, ftype, fid) = iprot.readFieldBegin()
            if ftype == TType.STOP:
                break
            if fid == 1:
                if ftype == TType.DOUBLE:
                    self.decisionMetric = iprot.readDouble()
                else:
                    iprot.skip(ftype)
            elif fid == 3:
                if ftype == TType.STRUCT:
                    self.resultantSpectrum = Spectrum.ttypes.SpectrumResult()
                    self.resultantSpectrum.read(iprot)
                else:
                    iprot.skip(ftype)
            elif fid == 4:
                if ftype == TType.BOOL:
                    self.inAlarm = iprot.readBool()
                else:
                    iprot.skip(ftype)
            elif fid == 5:
                if ftype == TType.DOUBLE:
                    self.gammaDose = iprot.readDouble()
                else:
                    iprot.skip(ftype)
            elif fid == 6:
                if ftype == TType.DOUBLE:
                    self.neutronDose = iprot.readDouble()
                else:
                    iprot.skip(ftype)
            else:
                iprot.skip(ftype)
            iprot.readFieldEnd()
        iprot.readStructEnd()

    def write(self, oprot):
        if oprot._fast_encode is not None and self.thrift_spec is not None:
            oprot.trans.write(oprot._fast_encode(self, [self.__class__, self.thrift_spec]))
            return
        oprot.writeStructBegin('DetectionResult')
        if self.decisionMetric is not None:
            oprot.writeFieldBegin('decisionMetric', TType.DOUBLE, 1)
            oprot.writeDouble(self.decisionMetric)
            oprot.writeFieldEnd()
        if self.resultantSpectrum is not None:
            oprot.writeFieldBegin('resultantSpectrum', TType.STRUCT, 3)
            self.resultantSpectrum.write(oprot)
            oprot.writeFieldEnd()
        if self.inAlarm is not None:
            oprot.writeFieldBegin('inAlarm', TType.BOOL, 4)
            oprot.writeBool(self.inAlarm)
            oprot.writeFieldEnd()
        if self.gammaDose is not None:
            oprot.writeFieldBegin('gammaDose', TType.DOUBLE, 5)
            oprot.writeDouble(self.gammaDose)
            oprot.writeFieldEnd()
        if self.neutronDose is not None:
            oprot.writeFieldBegin('neutronDose', TType.DOUBLE, 6)
            oprot.writeDouble(self.neutronDose)
            oprot.writeFieldEnd()
        oprot.writeFieldStop()
        oprot.writeStructEnd()

    def validate(self):
        return

    def __repr__(self):
        L = ['%s=%r' % (key, value)
             for key, value in self.__dict__.items()]
        return '%s(%s)' % (self.__class__.__name__, ', '.join(L))

    def __eq__(self, other):
        return isinstance(other, self.__class__) and self.__dict__ == other.__dict__

    def __ne__(self, other):
        return not (self == other)
all_structs.append(DetectionConfiguration)
DetectionConfiguration.thrift_spec = (
    None,  # 0
    (1, TType.LIST, 'nuclidesSensitivity', (TType.I32, None, False), None, ),  # 1
    (2, TType.I64, 'backgroundWindowSize', None, None, ),  # 2
    (3, TType.I64, 'foregroundWindowSize', None, None, ),  # 3
    (4, TType.DOUBLE, 'lowerAlarmThreshold', None, None, ),  # 4
    (5, TType.DOUBLE, 'upperAlarmThreshold', None, None, ),  # 5
    (6, TType.DOUBLE, 'lowerROIThreshold', None, None, ),  # 6
    (7, TType.DOUBLE, 'upperROIThreshold', None, None, ),  # 7
)
all_structs.append(DetectionDefinition)
DetectionDefinition.thrift_spec = (
    None,  # 0
    (1, TType.STRUCT, 'startingDetectionConfiguration', [DetectionConfiguration, None], None, ),  # 1
)
all_structs.append(DetectionResult)
DetectionResult.thrift_spec = (
    None,  # 0
    (1, TType.DOUBLE, 'decisionMetric', None, None, ),  # 1
    None,  # 2
    (3, TType.STRUCT, 'resultantSpectrum', [Spectrum.ttypes.SpectrumResult, None], None, ),  # 3
    (4, TType.BOOL, 'inAlarm', None, None, ),  # 4
    (5, TType.DOUBLE, 'gammaDose', None, None, ),  # 5
    (6, TType.DOUBLE, 'neutronDose', None, None, ),  # 6
)
fix_spec(all_structs)
del all_structs
