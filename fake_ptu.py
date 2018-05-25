
############### PTU STUFF ###############
import sys
import glob
sys.path.append('./WIND-Thrift/gen-py')
sys.path.insert(0, '/home/holiestcow/thrift-0.11.0/lib/py/build/lib.linux-x86_64-3.5')

from CVRSServices.CVRSEndpoint import Client
# from server import CVRSHandler
# from CVRSServices.ttypes import (StatusCode, ControlType, Session, StartRecordingControlPayload,
#                                  ControlPayloadUnion, ControlMessage, ControlMessageAck,
#                                  RecordingUpdate, DefinitionAndConfigurationUpdate)
from PTUPayload import UnitDefinition
from server import
from Exceptions.ttypes import InvalidSession

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.server import TServer
from thrift.protocol import TBinaryProtocol

import time

############ Database Stuff ##############

# from .thrift_uuid import *
from database import DatabaseOperations

########## DONE IMPORTING ################

start_clock = dt.datetime.now()

def main():
    # Make socket
    transport = TSocket.TSocket('127.0.0.1', 9090)

    # Buffering is critical. Raw sockets are very slow
    transport = TTransport.TBufferedTransport(transport)

    # Wrap in a protocol
    protocol = TBinaryProtocol.TBinaryProtocol(transport)

    # Create a client to use the protocol encoder
    client = CVRSHandler.Client(protocol)

    # Connect!
    transport.open()

    # Define what I am
    unit_definition = UnitDefinition(uuid=generate_thrift_uuid(),
                                     unitName='Fake_PTU_Unit',
                                     softwareVersion='0.1',
                                     hardwareRevision='0.1',
                                     vendor='University of Tennessee - Knoxville',
                                     unitType='Wearable',
                                     protocolVersion=PROTOCOL_VERSION)
    # Initiate handshake
    session = client.registerPtu(unitDefinition=unit_definition)
    time.sleep(5)

    # NOTE: This is where I am.

    client.define(session.sessionId, status, systemDefinition, systemConfiguration, recordingUpdate)


    # NOTE: Frankly I should never get to the close, since I'll be infinitely looping
    transport.close()
    return

if __name__ == '__main__':
    try:
        main()
    except Thrift.TException as tx:
        print('%s' % tx.message)
