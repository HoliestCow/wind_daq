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

from thrift.transport import TTransport
all_structs = []


class Health(object):
    Offline = 0
    Degraded = 1
    Nominal = 2

    _VALUES_TO_NAMES = {
        0: "Offline",
        1: "Degraded",
        2: "Nominal",
    }

    _NAMES_TO_VALUES = {
        "Offline": 0,
        "Degraded": 1,
        "Nominal": 2,
    }
fix_spec(all_structs)
del all_structs
