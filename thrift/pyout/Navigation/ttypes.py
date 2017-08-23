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
import UUID.ttypes

from thrift.transport import TTransport
all_structs = []


class BoundingBoxType(object):
    MissionArea = 0
    AreaOfInterest = 1
    Other = 2

    _VALUES_TO_NAMES = {
        0: "MissionArea",
        1: "AreaOfInterest",
        2: "Other",
    }

    _NAMES_TO_VALUES = {
        "MissionArea": 0,
        "AreaOfInterest": 1,
        "Other": 2,
    }


class Location(object):
    """
    Attributes:
     - latitude
     - longitude
     - altitude
    """


    def __init__(self, latitude=None, longitude=None, altitude=None,):
        self.latitude = latitude
        self.longitude = longitude
        self.altitude = altitude

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
                    self.latitude = iprot.readDouble()
                else:
                    iprot.skip(ftype)
            elif fid == 2:
                if ftype == TType.DOUBLE:
                    self.longitude = iprot.readDouble()
                else:
                    iprot.skip(ftype)
            elif fid == 3:
                if ftype == TType.DOUBLE:
                    self.altitude = iprot.readDouble()
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
        oprot.writeStructBegin('Location')
        if self.latitude is not None:
            oprot.writeFieldBegin('latitude', TType.DOUBLE, 1)
            oprot.writeDouble(self.latitude)
            oprot.writeFieldEnd()
        if self.longitude is not None:
            oprot.writeFieldBegin('longitude', TType.DOUBLE, 2)
            oprot.writeDouble(self.longitude)
            oprot.writeFieldEnd()
        if self.altitude is not None:
            oprot.writeFieldBegin('altitude', TType.DOUBLE, 3)
            oprot.writeDouble(self.altitude)
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


class Waypoint(object):
    """
    Attributes:
     - waypointId
     - timeStamp
     - name
     - location
    """


    def __init__(self, waypointId=None, timeStamp=None, name=None, location=None,):
        self.waypointId = waypointId
        self.timeStamp = timeStamp
        self.name = name
        self.location = location

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
                    self.waypointId = UUID.ttypes.UUID()
                    self.waypointId.read(iprot)
                else:
                    iprot.skip(ftype)
            elif fid == 2:
                if ftype == TType.I64:
                    self.timeStamp = iprot.readI64()
                else:
                    iprot.skip(ftype)
            elif fid == 3:
                if ftype == TType.STRING:
                    self.name = iprot.readString().decode('utf-8') if sys.version_info[0] == 2 else iprot.readString()
                else:
                    iprot.skip(ftype)
            elif fid == 4:
                if ftype == TType.STRUCT:
                    self.location = Location()
                    self.location.read(iprot)
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
        oprot.writeStructBegin('Waypoint')
        if self.waypointId is not None:
            oprot.writeFieldBegin('waypointId', TType.STRUCT, 1)
            self.waypointId.write(oprot)
            oprot.writeFieldEnd()
        if self.timeStamp is not None:
            oprot.writeFieldBegin('timeStamp', TType.I64, 2)
            oprot.writeI64(self.timeStamp)
            oprot.writeFieldEnd()
        if self.name is not None:
            oprot.writeFieldBegin('name', TType.STRING, 3)
            oprot.writeString(self.name.encode('utf-8') if sys.version_info[0] == 2 else self.name)
            oprot.writeFieldEnd()
        if self.location is not None:
            oprot.writeFieldBegin('location', TType.STRUCT, 4)
            self.location.write(oprot)
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


class BoundingBox(object):
    """
    Attributes:
     - boundingBoxId
     - timeStamp
     - name
     - type
     - vertices
    """


    def __init__(self, boundingBoxId=None, timeStamp=None, name=None, type=None, vertices=None,):
        self.boundingBoxId = boundingBoxId
        self.timeStamp = timeStamp
        self.name = name
        self.type = type
        self.vertices = vertices

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
                    self.boundingBoxId = UUID.ttypes.UUID()
                    self.boundingBoxId.read(iprot)
                else:
                    iprot.skip(ftype)
            elif fid == 2:
                if ftype == TType.I64:
                    self.timeStamp = iprot.readI64()
                else:
                    iprot.skip(ftype)
            elif fid == 3:
                if ftype == TType.STRING:
                    self.name = iprot.readString().decode('utf-8') if sys.version_info[0] == 2 else iprot.readString()
                else:
                    iprot.skip(ftype)
            elif fid == 4:
                if ftype == TType.I32:
                    self.type = iprot.readI32()
                else:
                    iprot.skip(ftype)
            elif fid == 5:
                if ftype == TType.LIST:
                    self.vertices = []
                    (_etype3, _size0) = iprot.readListBegin()
                    for _i4 in range(_size0):
                        _elem5 = Location()
                        _elem5.read(iprot)
                        self.vertices.append(_elem5)
                    iprot.readListEnd()
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
        oprot.writeStructBegin('BoundingBox')
        if self.boundingBoxId is not None:
            oprot.writeFieldBegin('boundingBoxId', TType.STRUCT, 1)
            self.boundingBoxId.write(oprot)
            oprot.writeFieldEnd()
        if self.timeStamp is not None:
            oprot.writeFieldBegin('timeStamp', TType.I64, 2)
            oprot.writeI64(self.timeStamp)
            oprot.writeFieldEnd()
        if self.name is not None:
            oprot.writeFieldBegin('name', TType.STRING, 3)
            oprot.writeString(self.name.encode('utf-8') if sys.version_info[0] == 2 else self.name)
            oprot.writeFieldEnd()
        if self.type is not None:
            oprot.writeFieldBegin('type', TType.I32, 4)
            oprot.writeI32(self.type)
            oprot.writeFieldEnd()
        if self.vertices is not None:
            oprot.writeFieldBegin('vertices', TType.LIST, 5)
            oprot.writeListBegin(TType.STRUCT, len(self.vertices))
            for iter6 in self.vertices:
                iter6.write(oprot)
            oprot.writeListEnd()
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
all_structs.append(Location)
Location.thrift_spec = (
    None,  # 0
    (1, TType.DOUBLE, 'latitude', None, None, ),  # 1
    (2, TType.DOUBLE, 'longitude', None, None, ),  # 2
    (3, TType.DOUBLE, 'altitude', None, None, ),  # 3
)
all_structs.append(Waypoint)
Waypoint.thrift_spec = (
    None,  # 0
    (1, TType.STRUCT, 'waypointId', [UUID.ttypes.UUID, None], None, ),  # 1
    (2, TType.I64, 'timeStamp', None, None, ),  # 2
    (3, TType.STRING, 'name', 'UTF8', None, ),  # 3
    (4, TType.STRUCT, 'location', [Location, None], None, ),  # 4
)
all_structs.append(BoundingBox)
BoundingBox.thrift_spec = (
    None,  # 0
    (1, TType.STRUCT, 'boundingBoxId', [UUID.ttypes.UUID, None], None, ),  # 1
    (2, TType.I64, 'timeStamp', None, None, ),  # 2
    (3, TType.STRING, 'name', 'UTF8', None, ),  # 3
    (4, TType.I32, 'type', None, None, ),  # 4
    (5, TType.LIST, 'vertices', (TType.STRUCT, [Location, None], False), None, ),  # 5
)
fix_spec(all_structs)
del all_structs
