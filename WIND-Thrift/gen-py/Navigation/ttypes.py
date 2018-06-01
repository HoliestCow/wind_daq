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
import UUID.ttypes
import Common.ttypes

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


class MapOverlayType(object):
    """
    Map overlays are used by PTU operators to aid them in localizing themselves and
    understanding their progress in completing tasks such as clearing an area. A
    floorplan may be provided by the CVRS or generated by algorithms as PTUs move through an area.
    A clearing map will show what area of interest remains to be cleared. Provision of such a map
    may be useful to sync information between PTUs (especially PTUs joining a task that is
    already underway).
    """
    Floorplan = 0
    ClearingMap = 1

    _VALUES_TO_NAMES = {
        0: "Floorplan",
        1: "ClearingMap",
    }

    _NAMES_TO_VALUES = {
        "Floorplan": 0,
        "ClearingMap": 1,
    }


class Location(object):
    """
    Attributes:
     - latitude: Latitude in decimal degrees
     - longitude: Longitude in decimal degrees
     - altitude: Altitude of the point in meters.
    If not specified then the surface of the Earth is assumed.
    Units are in meters.
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


class LocationUncertainty(object):
    """
    Description of the uncertainty in an absolute location.
    This is an elipsoid centered about the location expressed in
    2+ dimensions. The altitude uncertainty is
    not correlated with latitude or longitude.

    Attributes:
     - angle: Angle in degrees to rotate the majorUncertainty axis into the lat/long
    coordinate system.
    For example, if angle is 0, the majorUncertainty refers to uncertainty in
    latitude and minorUncertainty is in longitude. An angle of 90 indicates
    that majorUncertainty refers to longitude and minorUncertainty to latitude.
     - majorUncertainty: Uncertainty along major and minor axes of an ellipse.
    majorUncertainty must be greater than or equal to the minorUncertainty.
    Units are the standard deviation in decimal degrees.
     - minorUncertainty
     - altitudeUncertainty: Uncertainty in the altitude if available.
    Units are the standard deviation in meters.
    """


    def __init__(self, angle=None, majorUncertainty=None, minorUncertainty=None, altitudeUncertainty=None,):
        self.angle = angle
        self.majorUncertainty = majorUncertainty
        self.minorUncertainty = minorUncertainty
        self.altitudeUncertainty = altitudeUncertainty

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
                    self.angle = iprot.readDouble()
                else:
                    iprot.skip(ftype)
            elif fid == 2:
                if ftype == TType.DOUBLE:
                    self.majorUncertainty = iprot.readDouble()
                else:
                    iprot.skip(ftype)
            elif fid == 3:
                if ftype == TType.DOUBLE:
                    self.minorUncertainty = iprot.readDouble()
                else:
                    iprot.skip(ftype)
            elif fid == 4:
                if ftype == TType.DOUBLE:
                    self.altitudeUncertainty = iprot.readDouble()
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
        oprot.writeStructBegin('LocationUncertainty')
        if self.angle is not None:
            oprot.writeFieldBegin('angle', TType.DOUBLE, 1)
            oprot.writeDouble(self.angle)
            oprot.writeFieldEnd()
        if self.majorUncertainty is not None:
            oprot.writeFieldBegin('majorUncertainty', TType.DOUBLE, 2)
            oprot.writeDouble(self.majorUncertainty)
            oprot.writeFieldEnd()
        if self.minorUncertainty is not None:
            oprot.writeFieldBegin('minorUncertainty', TType.DOUBLE, 3)
            oprot.writeDouble(self.minorUncertainty)
            oprot.writeFieldEnd()
        if self.altitudeUncertainty is not None:
            oprot.writeFieldBegin('altitudeUncertainty', TType.DOUBLE, 4)
            oprot.writeDouble(self.altitudeUncertainty)
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


class MapOverlay(object):
    """
    Attributes:
     - mapOverlayId
     - timeStamp
     - name
     - location
     - width
     - height
     - type
     - image
     - mimeType
    """


    def __init__(self, mapOverlayId=None, timeStamp=None, name=None, location=None, width=None, height=None, type=None, image=None, mimeType=None,):
        self.mapOverlayId = mapOverlayId
        self.timeStamp = timeStamp
        self.name = name
        self.location = location
        self.width = width
        self.height = height
        self.type = type
        self.image = image
        self.mimeType = mimeType

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
                    self.mapOverlayId = UUID.ttypes.UUID()
                    self.mapOverlayId.read(iprot)
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
            elif fid == 5:
                if ftype == TType.DOUBLE:
                    self.width = iprot.readDouble()
                else:
                    iprot.skip(ftype)
            elif fid == 6:
                if ftype == TType.DOUBLE:
                    self.height = iprot.readDouble()
                else:
                    iprot.skip(ftype)
            elif fid == 7:
                if ftype == TType.I32:
                    self.type = iprot.readI32()
                else:
                    iprot.skip(ftype)
            elif fid == 8:
                if ftype == TType.STRING:
                    self.image = iprot.readBinary()
                else:
                    iprot.skip(ftype)
            elif fid == 9:
                if ftype == TType.STRING:
                    self.mimeType = iprot.readString().decode('utf-8') if sys.version_info[0] == 2 else iprot.readString()
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
        oprot.writeStructBegin('MapOverlay')
        if self.mapOverlayId is not None:
            oprot.writeFieldBegin('mapOverlayId', TType.STRUCT, 1)
            self.mapOverlayId.write(oprot)
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
        if self.width is not None:
            oprot.writeFieldBegin('width', TType.DOUBLE, 5)
            oprot.writeDouble(self.width)
            oprot.writeFieldEnd()
        if self.height is not None:
            oprot.writeFieldBegin('height', TType.DOUBLE, 6)
            oprot.writeDouble(self.height)
            oprot.writeFieldEnd()
        if self.type is not None:
            oprot.writeFieldBegin('type', TType.I32, 7)
            oprot.writeI32(self.type)
            oprot.writeFieldEnd()
        if self.image is not None:
            oprot.writeFieldBegin('image', TType.STRING, 8)
            oprot.writeBinary(self.image)
            oprot.writeFieldEnd()
        if self.mimeType is not None:
            oprot.writeFieldBegin('mimeType', TType.STRING, 9)
            oprot.writeString(self.mimeType.encode('utf-8') if sys.version_info[0] == 2 else self.mimeType)
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
all_structs.append(LocationUncertainty)
LocationUncertainty.thrift_spec = (
    None,  # 0
    (1, TType.DOUBLE, 'angle', None, None, ),  # 1
    (2, TType.DOUBLE, 'majorUncertainty', None, None, ),  # 2
    (3, TType.DOUBLE, 'minorUncertainty', None, None, ),  # 3
    (4, TType.DOUBLE, 'altitudeUncertainty', None, None, ),  # 4
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
all_structs.append(MapOverlay)
MapOverlay.thrift_spec = (
    None,  # 0
    (1, TType.STRUCT, 'mapOverlayId', [UUID.ttypes.UUID, None], None, ),  # 1
    (2, TType.I64, 'timeStamp', None, None, ),  # 2
    (3, TType.STRING, 'name', 'UTF8', None, ),  # 3
    (4, TType.STRUCT, 'location', [Location, None], None, ),  # 4
    (5, TType.DOUBLE, 'width', None, None, ),  # 5
    (6, TType.DOUBLE, 'height', None, None, ),  # 6
    (7, TType.I32, 'type', None, None, ),  # 7
    (8, TType.STRING, 'image', 'BINARY', None, ),  # 8
    (9, TType.STRING, 'mimeType', 'UTF8', None, ),  # 9
)
fix_spec(all_structs)
del all_structs