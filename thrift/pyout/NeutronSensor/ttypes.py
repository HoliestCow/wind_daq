#
# Autogenerated by Thrift Compiler (1.0.0-dev)
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#
#  options string: py
#

from thrift.Thrift import TType, TMessageType, TFrozenDict, TException, TApplicationException
from thrift.protocol.TProtocol import TProtocolException
from thrift.TRecursive import fix_spec

import sys
import Health.ttypes
import DetectorCharacteristics.ttypes
import Spectrum.ttypes
import ComponentLocation.ttypes
import UUID.ttypes

from thrift.transport import TTransport
all_structs = []


class NeutronConfiguration(object):
    """
    Attributes:
     - componentId
     - componentPositionAndOrientation
    """


    def __init__(self, componentId=None, componentPositionAndOrientation=None,):
        self.componentId = componentId
        self.componentPositionAndOrientation = componentPositionAndOrientation

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
                    self.componentId = UUID.ttypes.UUID()
                    self.componentId.read(iprot)
                else:
                    iprot.skip(ftype)
            elif fid == 2:
                if ftype == TType.STRUCT:
                    self.componentPositionAndOrientation = ComponentLocation.ttypes.GridPositionAndOrientation()
                    self.componentPositionAndOrientation.read(iprot)
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
        oprot.writeStructBegin('NeutronConfiguration')
        if self.componentId is not None:
            oprot.writeFieldBegin('componentId', TType.STRUCT, 1)
            self.componentId.write(oprot)
            oprot.writeFieldEnd()
        if self.componentPositionAndOrientation is not None:
            oprot.writeFieldBegin('componentPositionAndOrientation', TType.STRUCT, 2)
            self.componentPositionAndOrientation.write(oprot)
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


class NeutronListDefinition(object):
    """
    Attributes:
     - componentId
     - componentName
     - vendorName
     - serialNumber
     - numberOfChannels
     - physicalDimensions
     - detectorMaterial
     - startingListNeutronConfiguration
    """


    def __init__(self, componentId=None, componentName=None, vendorName=None, serialNumber=None, numberOfChannels=None, physicalDimensions=None, detectorMaterial=None, startingListNeutronConfiguration=None,):
        self.componentId = componentId
        self.componentName = componentName
        self.vendorName = vendorName
        self.serialNumber = serialNumber
        self.numberOfChannels = numberOfChannels
        self.physicalDimensions = physicalDimensions
        self.detectorMaterial = detectorMaterial
        self.startingListNeutronConfiguration = startingListNeutronConfiguration

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
                    self.componentId = UUID.ttypes.UUID()
                    self.componentId.read(iprot)
                else:
                    iprot.skip(ftype)
            elif fid == 2:
                if ftype == TType.STRING:
                    self.componentName = iprot.readString().decode('utf-8') if sys.version_info[0] == 2 else iprot.readString()
                else:
                    iprot.skip(ftype)
            elif fid == 3:
                if ftype == TType.STRING:
                    self.vendorName = iprot.readString().decode('utf-8') if sys.version_info[0] == 2 else iprot.readString()
                else:
                    iprot.skip(ftype)
            elif fid == 4:
                if ftype == TType.STRING:
                    self.serialNumber = iprot.readString().decode('utf-8') if sys.version_info[0] == 2 else iprot.readString()
                else:
                    iprot.skip(ftype)
            elif fid == 5:
                if ftype == TType.I32:
                    self.numberOfChannels = iprot.readI32()
                else:
                    iprot.skip(ftype)
            elif fid == 6:
                if ftype == TType.STRUCT:
                    self.physicalDimensions = DetectorCharacteristics.ttypes.Dimensions()
                    self.physicalDimensions.read(iprot)
                else:
                    iprot.skip(ftype)
            elif fid == 7:
                if ftype == TType.I32:
                    self.detectorMaterial = iprot.readI32()
                else:
                    iprot.skip(ftype)
            elif fid == 8:
                if ftype == TType.STRUCT:
                    self.startingListNeutronConfiguration = NeutronConfiguration()
                    self.startingListNeutronConfiguration.read(iprot)
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
        oprot.writeStructBegin('NeutronListDefinition')
        if self.componentId is not None:
            oprot.writeFieldBegin('componentId', TType.STRUCT, 1)
            self.componentId.write(oprot)
            oprot.writeFieldEnd()
        if self.componentName is not None:
            oprot.writeFieldBegin('componentName', TType.STRING, 2)
            oprot.writeString(self.componentName.encode('utf-8') if sys.version_info[0] == 2 else self.componentName)
            oprot.writeFieldEnd()
        if self.vendorName is not None:
            oprot.writeFieldBegin('vendorName', TType.STRING, 3)
            oprot.writeString(self.vendorName.encode('utf-8') if sys.version_info[0] == 2 else self.vendorName)
            oprot.writeFieldEnd()
        if self.serialNumber is not None:
            oprot.writeFieldBegin('serialNumber', TType.STRING, 4)
            oprot.writeString(self.serialNumber.encode('utf-8') if sys.version_info[0] == 2 else self.serialNumber)
            oprot.writeFieldEnd()
        if self.numberOfChannels is not None:
            oprot.writeFieldBegin('numberOfChannels', TType.I32, 5)
            oprot.writeI32(self.numberOfChannels)
            oprot.writeFieldEnd()
        if self.physicalDimensions is not None:
            oprot.writeFieldBegin('physicalDimensions', TType.STRUCT, 6)
            self.physicalDimensions.write(oprot)
            oprot.writeFieldEnd()
        if self.detectorMaterial is not None:
            oprot.writeFieldBegin('detectorMaterial', TType.I32, 7)
            oprot.writeI32(self.detectorMaterial)
            oprot.writeFieldEnd()
        if self.startingListNeutronConfiguration is not None:
            oprot.writeFieldBegin('startingListNeutronConfiguration', TType.STRUCT, 8)
            self.startingListNeutronConfiguration.write(oprot)
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


class NeutronListData(object):
    """
    Attributes:
     - componentId
     - timeStamp: Milliseconds since UNIX epoch
     - health
     - listModeData
     - liveTime
     - realTime
     - ListNeutronConfiguration
    """


    def __init__(self, componentId=None, timeStamp=None, health=None, listModeData=None, liveTime=None, realTime=None, ListNeutronConfiguration=None,):
        self.componentId = componentId
        self.timeStamp = timeStamp
        self.health = health
        self.listModeData = listModeData
        self.liveTime = liveTime
        self.realTime = realTime
        self.ListNeutronConfiguration = ListNeutronConfiguration

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
                    self.componentId = UUID.ttypes.UUID()
                    self.componentId.read(iprot)
                else:
                    iprot.skip(ftype)
            elif fid == 2:
                if ftype == TType.I64:
                    self.timeStamp = iprot.readI64()
                else:
                    iprot.skip(ftype)
            elif fid == 3:
                if ftype == TType.I32:
                    self.health = iprot.readI32()
                else:
                    iprot.skip(ftype)
            elif fid == 4:
                if ftype == TType.LIST:
                    self.listModeData = []
                    (_etype3, _size0) = iprot.readListBegin()
                    for _i4 in range(_size0):
                        _elem5 = Spectrum.ttypes.ListMode()
                        _elem5.read(iprot)
                        self.listModeData.append(_elem5)
                    iprot.readListEnd()
                else:
                    iprot.skip(ftype)
            elif fid == 5:
                if ftype == TType.I64:
                    self.liveTime = iprot.readI64()
                else:
                    iprot.skip(ftype)
            elif fid == 6:
                if ftype == TType.I64:
                    self.realTime = iprot.readI64()
                else:
                    iprot.skip(ftype)
            elif fid == 7:
                if ftype == TType.STRUCT:
                    self.ListNeutronConfiguration = NeutronConfiguration()
                    self.ListNeutronConfiguration.read(iprot)
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
        oprot.writeStructBegin('NeutronListData')
        if self.componentId is not None:
            oprot.writeFieldBegin('componentId', TType.STRUCT, 1)
            self.componentId.write(oprot)
            oprot.writeFieldEnd()
        if self.timeStamp is not None:
            oprot.writeFieldBegin('timeStamp', TType.I64, 2)
            oprot.writeI64(self.timeStamp)
            oprot.writeFieldEnd()
        if self.health is not None:
            oprot.writeFieldBegin('health', TType.I32, 3)
            oprot.writeI32(self.health)
            oprot.writeFieldEnd()
        if self.listModeData is not None:
            oprot.writeFieldBegin('listModeData', TType.LIST, 4)
            oprot.writeListBegin(TType.STRUCT, len(self.listModeData))
            for iter6 in self.listModeData:
                iter6.write(oprot)
            oprot.writeListEnd()
            oprot.writeFieldEnd()
        if self.liveTime is not None:
            oprot.writeFieldBegin('liveTime', TType.I64, 5)
            oprot.writeI64(self.liveTime)
            oprot.writeFieldEnd()
        if self.realTime is not None:
            oprot.writeFieldBegin('realTime', TType.I64, 6)
            oprot.writeI64(self.realTime)
            oprot.writeFieldEnd()
        if self.ListNeutronConfiguration is not None:
            oprot.writeFieldBegin('ListNeutronConfiguration', TType.STRUCT, 7)
            self.ListNeutronConfiguration.write(oprot)
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


class NeutronSpectrumDefinition(object):
    """
    Attributes:
     - componentId
     - componentName
     - vendorName
     - serialNumber
     - numberOfChannels
     - physicalDimensions
     - detectorMaterial
     - startingSpectrumNeutronConfiguration
    """


    def __init__(self, componentId=None, componentName=None, vendorName=None, serialNumber=None, numberOfChannels=None, physicalDimensions=None, detectorMaterial=None, startingSpectrumNeutronConfiguration=None,):
        self.componentId = componentId
        self.componentName = componentName
        self.vendorName = vendorName
        self.serialNumber = serialNumber
        self.numberOfChannels = numberOfChannels
        self.physicalDimensions = physicalDimensions
        self.detectorMaterial = detectorMaterial
        self.startingSpectrumNeutronConfiguration = startingSpectrumNeutronConfiguration

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
                    self.componentId = UUID.ttypes.UUID()
                    self.componentId.read(iprot)
                else:
                    iprot.skip(ftype)
            elif fid == 2:
                if ftype == TType.STRING:
                    self.componentName = iprot.readString().decode('utf-8') if sys.version_info[0] == 2 else iprot.readString()
                else:
                    iprot.skip(ftype)
            elif fid == 3:
                if ftype == TType.STRING:
                    self.vendorName = iprot.readString().decode('utf-8') if sys.version_info[0] == 2 else iprot.readString()
                else:
                    iprot.skip(ftype)
            elif fid == 4:
                if ftype == TType.STRING:
                    self.serialNumber = iprot.readString().decode('utf-8') if sys.version_info[0] == 2 else iprot.readString()
                else:
                    iprot.skip(ftype)
            elif fid == 5:
                if ftype == TType.I32:
                    self.numberOfChannels = iprot.readI32()
                else:
                    iprot.skip(ftype)
            elif fid == 6:
                if ftype == TType.STRUCT:
                    self.physicalDimensions = DetectorCharacteristics.ttypes.Dimensions()
                    self.physicalDimensions.read(iprot)
                else:
                    iprot.skip(ftype)
            elif fid == 7:
                if ftype == TType.I32:
                    self.detectorMaterial = iprot.readI32()
                else:
                    iprot.skip(ftype)
            elif fid == 8:
                if ftype == TType.STRUCT:
                    self.startingSpectrumNeutronConfiguration = NeutronConfiguration()
                    self.startingSpectrumNeutronConfiguration.read(iprot)
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
        oprot.writeStructBegin('NeutronSpectrumDefinition')
        if self.componentId is not None:
            oprot.writeFieldBegin('componentId', TType.STRUCT, 1)
            self.componentId.write(oprot)
            oprot.writeFieldEnd()
        if self.componentName is not None:
            oprot.writeFieldBegin('componentName', TType.STRING, 2)
            oprot.writeString(self.componentName.encode('utf-8') if sys.version_info[0] == 2 else self.componentName)
            oprot.writeFieldEnd()
        if self.vendorName is not None:
            oprot.writeFieldBegin('vendorName', TType.STRING, 3)
            oprot.writeString(self.vendorName.encode('utf-8') if sys.version_info[0] == 2 else self.vendorName)
            oprot.writeFieldEnd()
        if self.serialNumber is not None:
            oprot.writeFieldBegin('serialNumber', TType.STRING, 4)
            oprot.writeString(self.serialNumber.encode('utf-8') if sys.version_info[0] == 2 else self.serialNumber)
            oprot.writeFieldEnd()
        if self.numberOfChannels is not None:
            oprot.writeFieldBegin('numberOfChannels', TType.I32, 5)
            oprot.writeI32(self.numberOfChannels)
            oprot.writeFieldEnd()
        if self.physicalDimensions is not None:
            oprot.writeFieldBegin('physicalDimensions', TType.STRUCT, 6)
            self.physicalDimensions.write(oprot)
            oprot.writeFieldEnd()
        if self.detectorMaterial is not None:
            oprot.writeFieldBegin('detectorMaterial', TType.I32, 7)
            oprot.writeI32(self.detectorMaterial)
            oprot.writeFieldEnd()
        if self.startingSpectrumNeutronConfiguration is not None:
            oprot.writeFieldBegin('startingSpectrumNeutronConfiguration', TType.STRUCT, 8)
            self.startingSpectrumNeutronConfiguration.write(oprot)
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


class NeutronSpectrumData(object):
    """
    Attributes:
     - componentId
     - timeStamp: Milliseconds since UNIX epoch
     - health
     - spectrum
     - liveTime
     - realTime
     - neutronSpectrumConfiguration
    """


    def __init__(self, componentId=None, timeStamp=None, health=None, spectrum=None, liveTime=None, realTime=None, neutronSpectrumConfiguration=None,):
        self.componentId = componentId
        self.timeStamp = timeStamp
        self.health = health
        self.spectrum = spectrum
        self.liveTime = liveTime
        self.realTime = realTime
        self.neutronSpectrumConfiguration = neutronSpectrumConfiguration

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
                    self.componentId = UUID.ttypes.UUID()
                    self.componentId.read(iprot)
                else:
                    iprot.skip(ftype)
            elif fid == 2:
                if ftype == TType.I64:
                    self.timeStamp = iprot.readI64()
                else:
                    iprot.skip(ftype)
            elif fid == 3:
                if ftype == TType.I32:
                    self.health = iprot.readI32()
                else:
                    iprot.skip(ftype)
            elif fid == 4:
                if ftype == TType.STRUCT:
                    self.spectrum = Spectrum.ttypes.SpectrumResult()
                    self.spectrum.read(iprot)
                else:
                    iprot.skip(ftype)
            elif fid == 5:
                if ftype == TType.I64:
                    self.liveTime = iprot.readI64()
                else:
                    iprot.skip(ftype)
            elif fid == 6:
                if ftype == TType.I64:
                    self.realTime = iprot.readI64()
                else:
                    iprot.skip(ftype)
            elif fid == 7:
                if ftype == TType.STRUCT:
                    self.neutronSpectrumConfiguration = NeutronConfiguration()
                    self.neutronSpectrumConfiguration.read(iprot)
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
        oprot.writeStructBegin('NeutronSpectrumData')
        if self.componentId is not None:
            oprot.writeFieldBegin('componentId', TType.STRUCT, 1)
            self.componentId.write(oprot)
            oprot.writeFieldEnd()
        if self.timeStamp is not None:
            oprot.writeFieldBegin('timeStamp', TType.I64, 2)
            oprot.writeI64(self.timeStamp)
            oprot.writeFieldEnd()
        if self.health is not None:
            oprot.writeFieldBegin('health', TType.I32, 3)
            oprot.writeI32(self.health)
            oprot.writeFieldEnd()
        if self.spectrum is not None:
            oprot.writeFieldBegin('spectrum', TType.STRUCT, 4)
            self.spectrum.write(oprot)
            oprot.writeFieldEnd()
        if self.liveTime is not None:
            oprot.writeFieldBegin('liveTime', TType.I64, 5)
            oprot.writeI64(self.liveTime)
            oprot.writeFieldEnd()
        if self.realTime is not None:
            oprot.writeFieldBegin('realTime', TType.I64, 6)
            oprot.writeI64(self.realTime)
            oprot.writeFieldEnd()
        if self.neutronSpectrumConfiguration is not None:
            oprot.writeFieldBegin('neutronSpectrumConfiguration', TType.STRUCT, 7)
            self.neutronSpectrumConfiguration.write(oprot)
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


class NeutronGrossCountDefinition(object):
    """
    Attributes:
     - componentId
     - componentName
     - vendorName
     - serialNumber
     - physicalDimensions
     - detectorMaterial
     - startingNeutronConfiguration
    """


    def __init__(self, componentId=None, componentName=None, vendorName=None, serialNumber=None, physicalDimensions=None, detectorMaterial=None, startingNeutronConfiguration=None,):
        self.componentId = componentId
        self.componentName = componentName
        self.vendorName = vendorName
        self.serialNumber = serialNumber
        self.physicalDimensions = physicalDimensions
        self.detectorMaterial = detectorMaterial
        self.startingNeutronConfiguration = startingNeutronConfiguration

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
                    self.componentId = UUID.ttypes.UUID()
                    self.componentId.read(iprot)
                else:
                    iprot.skip(ftype)
            elif fid == 2:
                if ftype == TType.STRING:
                    self.componentName = iprot.readString().decode('utf-8') if sys.version_info[0] == 2 else iprot.readString()
                else:
                    iprot.skip(ftype)
            elif fid == 3:
                if ftype == TType.STRING:
                    self.vendorName = iprot.readString().decode('utf-8') if sys.version_info[0] == 2 else iprot.readString()
                else:
                    iprot.skip(ftype)
            elif fid == 4:
                if ftype == TType.STRING:
                    self.serialNumber = iprot.readString().decode('utf-8') if sys.version_info[0] == 2 else iprot.readString()
                else:
                    iprot.skip(ftype)
            elif fid == 5:
                if ftype == TType.STRUCT:
                    self.physicalDimensions = DetectorCharacteristics.ttypes.Dimensions()
                    self.physicalDimensions.read(iprot)
                else:
                    iprot.skip(ftype)
            elif fid == 6:
                if ftype == TType.I32:
                    self.detectorMaterial = iprot.readI32()
                else:
                    iprot.skip(ftype)
            elif fid == 7:
                if ftype == TType.STRUCT:
                    self.startingNeutronConfiguration = NeutronConfiguration()
                    self.startingNeutronConfiguration.read(iprot)
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
        oprot.writeStructBegin('NeutronGrossCountDefinition')
        if self.componentId is not None:
            oprot.writeFieldBegin('componentId', TType.STRUCT, 1)
            self.componentId.write(oprot)
            oprot.writeFieldEnd()
        if self.componentName is not None:
            oprot.writeFieldBegin('componentName', TType.STRING, 2)
            oprot.writeString(self.componentName.encode('utf-8') if sys.version_info[0] == 2 else self.componentName)
            oprot.writeFieldEnd()
        if self.vendorName is not None:
            oprot.writeFieldBegin('vendorName', TType.STRING, 3)
            oprot.writeString(self.vendorName.encode('utf-8') if sys.version_info[0] == 2 else self.vendorName)
            oprot.writeFieldEnd()
        if self.serialNumber is not None:
            oprot.writeFieldBegin('serialNumber', TType.STRING, 4)
            oprot.writeString(self.serialNumber.encode('utf-8') if sys.version_info[0] == 2 else self.serialNumber)
            oprot.writeFieldEnd()
        if self.physicalDimensions is not None:
            oprot.writeFieldBegin('physicalDimensions', TType.STRUCT, 5)
            self.physicalDimensions.write(oprot)
            oprot.writeFieldEnd()
        if self.detectorMaterial is not None:
            oprot.writeFieldBegin('detectorMaterial', TType.I32, 6)
            oprot.writeI32(self.detectorMaterial)
            oprot.writeFieldEnd()
        if self.startingNeutronConfiguration is not None:
            oprot.writeFieldBegin('startingNeutronConfiguration', TType.STRUCT, 7)
            self.startingNeutronConfiguration.write(oprot)
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


class NeutronGrossCountData(object):
    """
    Attributes:
     - componentId
     - timeStamp: Milliseconds since UNIX epoch
     - health
     - counts
     - liveTime
     - realTime
     - neutronGrossCountConfiguration
    """


    def __init__(self, componentId=None, timeStamp=None, health=None, counts=None, liveTime=None, realTime=None, neutronGrossCountConfiguration=None,):
        self.componentId = componentId
        self.timeStamp = timeStamp
        self.health = health
        self.counts = counts
        self.liveTime = liveTime
        self.realTime = realTime
        self.neutronGrossCountConfiguration = neutronGrossCountConfiguration

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
                    self.componentId = UUID.ttypes.UUID()
                    self.componentId.read(iprot)
                else:
                    iprot.skip(ftype)
            elif fid == 2:
                if ftype == TType.I64:
                    self.timeStamp = iprot.readI64()
                else:
                    iprot.skip(ftype)
            elif fid == 3:
                if ftype == TType.I32:
                    self.health = iprot.readI32()
                else:
                    iprot.skip(ftype)
            elif fid == 4:
                if ftype == TType.I32:
                    self.counts = iprot.readI32()
                else:
                    iprot.skip(ftype)
            elif fid == 5:
                if ftype == TType.I64:
                    self.liveTime = iprot.readI64()
                else:
                    iprot.skip(ftype)
            elif fid == 6:
                if ftype == TType.I64:
                    self.realTime = iprot.readI64()
                else:
                    iprot.skip(ftype)
            elif fid == 7:
                if ftype == TType.STRUCT:
                    self.neutronGrossCountConfiguration = NeutronConfiguration()
                    self.neutronGrossCountConfiguration.read(iprot)
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
        oprot.writeStructBegin('NeutronGrossCountData')
        if self.componentId is not None:
            oprot.writeFieldBegin('componentId', TType.STRUCT, 1)
            self.componentId.write(oprot)
            oprot.writeFieldEnd()
        if self.timeStamp is not None:
            oprot.writeFieldBegin('timeStamp', TType.I64, 2)
            oprot.writeI64(self.timeStamp)
            oprot.writeFieldEnd()
        if self.health is not None:
            oprot.writeFieldBegin('health', TType.I32, 3)
            oprot.writeI32(self.health)
            oprot.writeFieldEnd()
        if self.counts is not None:
            oprot.writeFieldBegin('counts', TType.I32, 4)
            oprot.writeI32(self.counts)
            oprot.writeFieldEnd()
        if self.liveTime is not None:
            oprot.writeFieldBegin('liveTime', TType.I64, 5)
            oprot.writeI64(self.liveTime)
            oprot.writeFieldEnd()
        if self.realTime is not None:
            oprot.writeFieldBegin('realTime', TType.I64, 6)
            oprot.writeI64(self.realTime)
            oprot.writeFieldEnd()
        if self.neutronGrossCountConfiguration is not None:
            oprot.writeFieldBegin('neutronGrossCountConfiguration', TType.STRUCT, 7)
            self.neutronGrossCountConfiguration.write(oprot)
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
all_structs.append(NeutronConfiguration)
NeutronConfiguration.thrift_spec = (
    None,  # 0
    (1, TType.STRUCT, 'componentId', [UUID.ttypes.UUID, None], None, ),  # 1
    (2, TType.STRUCT, 'componentPositionAndOrientation', [ComponentLocation.ttypes.GridPositionAndOrientation, None], None, ),  # 2
)
all_structs.append(NeutronListDefinition)
NeutronListDefinition.thrift_spec = (
    None,  # 0
    (1, TType.STRUCT, 'componentId', [UUID.ttypes.UUID, None], None, ),  # 1
    (2, TType.STRING, 'componentName', 'UTF8', None, ),  # 2
    (3, TType.STRING, 'vendorName', 'UTF8', None, ),  # 3
    (4, TType.STRING, 'serialNumber', 'UTF8', None, ),  # 4
    (5, TType.I32, 'numberOfChannels', None, None, ),  # 5
    (6, TType.STRUCT, 'physicalDimensions', [DetectorCharacteristics.ttypes.Dimensions, None], None, ),  # 6
    (7, TType.I32, 'detectorMaterial', None, None, ),  # 7
    (8, TType.STRUCT, 'startingListNeutronConfiguration', [NeutronConfiguration, None], None, ),  # 8
)
all_structs.append(NeutronListData)
NeutronListData.thrift_spec = (
    None,  # 0
    (1, TType.STRUCT, 'componentId', [UUID.ttypes.UUID, None], None, ),  # 1
    (2, TType.I64, 'timeStamp', None, None, ),  # 2
    (3, TType.I32, 'health', None, None, ),  # 3
    (4, TType.LIST, 'listModeData', (TType.STRUCT, [Spectrum.ttypes.ListMode, None], False), None, ),  # 4
    (5, TType.I64, 'liveTime', None, None, ),  # 5
    (6, TType.I64, 'realTime', None, None, ),  # 6
    (7, TType.STRUCT, 'ListNeutronConfiguration', [NeutronConfiguration, None], None, ),  # 7
)
all_structs.append(NeutronSpectrumDefinition)
NeutronSpectrumDefinition.thrift_spec = (
    None,  # 0
    (1, TType.STRUCT, 'componentId', [UUID.ttypes.UUID, None], None, ),  # 1
    (2, TType.STRING, 'componentName', 'UTF8', None, ),  # 2
    (3, TType.STRING, 'vendorName', 'UTF8', None, ),  # 3
    (4, TType.STRING, 'serialNumber', 'UTF8', None, ),  # 4
    (5, TType.I32, 'numberOfChannels', None, None, ),  # 5
    (6, TType.STRUCT, 'physicalDimensions', [DetectorCharacteristics.ttypes.Dimensions, None], None, ),  # 6
    (7, TType.I32, 'detectorMaterial', None, None, ),  # 7
    (8, TType.STRUCT, 'startingSpectrumNeutronConfiguration', [NeutronConfiguration, None], None, ),  # 8
)
all_structs.append(NeutronSpectrumData)
NeutronSpectrumData.thrift_spec = (
    None,  # 0
    (1, TType.STRUCT, 'componentId', [UUID.ttypes.UUID, None], None, ),  # 1
    (2, TType.I64, 'timeStamp', None, None, ),  # 2
    (3, TType.I32, 'health', None, None, ),  # 3
    (4, TType.STRUCT, 'spectrum', [Spectrum.ttypes.SpectrumResult, None], None, ),  # 4
    (5, TType.I64, 'liveTime', None, None, ),  # 5
    (6, TType.I64, 'realTime', None, None, ),  # 6
    (7, TType.STRUCT, 'neutronSpectrumConfiguration', [NeutronConfiguration, None], None, ),  # 7
)
all_structs.append(NeutronGrossCountDefinition)
NeutronGrossCountDefinition.thrift_spec = (
    None,  # 0
    (1, TType.STRUCT, 'componentId', [UUID.ttypes.UUID, None], None, ),  # 1
    (2, TType.STRING, 'componentName', 'UTF8', None, ),  # 2
    (3, TType.STRING, 'vendorName', 'UTF8', None, ),  # 3
    (4, TType.STRING, 'serialNumber', 'UTF8', None, ),  # 4
    (5, TType.STRUCT, 'physicalDimensions', [DetectorCharacteristics.ttypes.Dimensions, None], None, ),  # 5
    (6, TType.I32, 'detectorMaterial', None, None, ),  # 6
    (7, TType.STRUCT, 'startingNeutronConfiguration', [NeutronConfiguration, None], None, ),  # 7
)
all_structs.append(NeutronGrossCountData)
NeutronGrossCountData.thrift_spec = (
    None,  # 0
    (1, TType.STRUCT, 'componentId', [UUID.ttypes.UUID, None], None, ),  # 1
    (2, TType.I64, 'timeStamp', None, None, ),  # 2
    (3, TType.I32, 'health', None, None, ),  # 3
    (4, TType.I32, 'counts', None, None, ),  # 4
    (5, TType.I64, 'liveTime', None, None, ),  # 5
    (6, TType.I64, 'realTime', None, None, ),  # 6
    (7, TType.STRUCT, 'neutronGrossCountConfiguration', [NeutronConfiguration, None], None, ),  # 7
)
fix_spec(all_structs)
del all_structs
