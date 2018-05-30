
############### CVRS STUFF ###############
import sys
import glob
sys.path.append('./WIND-Thrift/gen-py')
sys.path.insert(0, '/home/holiestcow/thrift-0.11.0/lib/py/build/lib.linux-x86_64-3.5')

# from tutorial import Calculator
# from tutorial.ttypes import InvalidOperation, Operation, Work
import CVRSServices.CVRSEndpoint
from CVRSServices.CVRSEndpoint import Iface
from CVRSServices.ttypes import (StatusCode, ControlType, Session, StartRecordingControlPayload,
                                 ControlPayloadUnion, ControlMessage, ControlMessageAck,
                                 RecordingUpdate, DefinitionAndConfigurationUpdate)
from Exceptions.ttypes import InvalidSession
from UUID.ttypes import UUID
from PTUPayload.ttypes import Status

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.server import TServer
from thrift.protocol import TBinaryProtocol

############ Database Stuff ##############

# from .thrift_uuid import *
from database import DatabaseOperations
from thrift_uuid import Thrift_UUID
import time

########## DONE IMPORTING ################

start_clock = int(time.time() * 1000)

class CVRSHandler(Iface):
    """
    CVRSEndpoint

    This is the service implemented by WIND-compliant CVRS
    software, and provides a mechanism for the PTU to push
    data, status, and other information to the CVRS.

    This service defines a basic flow for connecting to a
    CVRS and sending it data. Each time a PTU re-establishes
    connection to the CVRS, the PTU MUST call these methods
    in the following order:

    1. registerPtu
    2. define
    loop (1hz):
        3. reportStatus
        4. pushData
        5. pushAcknowledgements

    The initial registration establishes a session between the PTU
    and the CVRS which can be used to track the state of the connection.
    The CVRS MAY determine that a session has become invalid if it has
    not received a message from the PTU within a reasonable period of time.

    The specific semantics of each method are described in the
    comments on the method definitions below.
    *
    """
    def __init__(self):
        self.log = {}
        self.sessions = {}
        self.registeredPTUs = {}
        self.PTUdefinitions = {}
        self.current_sessionId = None
        self.controlmessage_acknowledgements = []
        self.controlmessages = []
        self.db = None
        self.acknowledgements = []

    def _set_database(self, filename):
        self.db = DatabaseOperations(filename)
        return

    def ping(self):
        print('ping')
        return "pong"

    def registerPtu(self, unitDefinition):
        """
        Purpose: store PTU unitdefinition accessed by UUID.
        Parameters:
         - unitDefinition
        """
        self.registeredPTUs[unitDefinition.unitName] = unitDefinition
        x = Thrift_UUID.generate_thrift_uuid()
        newSessionId = UUID(
            leastSignificantBits=x[0],
            mostSignificantBits=x[1])
        self.current_sessionId = newSessionId
        return Session(status=StatusCode.OK, sessionId=newSessionId)

    def define(self, sessionId, status, systemDefinition, systemConfiguration, recordingUpdate):
        """
        Parameters:
         - sessionId
         - status
         - systemDefinition
         - systemConfiguration
         - recordingUpdate
        """
        # Check if the session matches the current session:
        # this may just be sessionId. I'm not sure if this is a Session object or not. (sessionId.sessionId)
        if sessionId == self.current_sessionId:
            ptu_message = []
            self.db.initialize_structure(numdetectors=1)
            return ptu_message
        else:
            # Sessions ID does not match.
            message = 'PTU Session ID {} does not match current session ID {}'.format(sessionId,
                self.current_sessionId)
            raise InvalidSession(sessionId=sessionId, message=message)
            ptu_message = []
            return ptu_message

    def reportStatus(self, sessionId, status, definitionAndConfigurationUpdate):
        # throws (1: Exceptions.InvalidSession error);
        # return [controlmessage]
        print('reporting')
        if sessionId == self.current_sessionId:
            message = []
            return message
        else:
            message = 'PTU Session ID {} does not match current session ID {}'.format(sessionId,
                self.current_sessionId)
            raise InvalidSession(sessionId=sessionId, message=message)
            message = []
            return message

    def pushData(self, sessionId, datum, definitionAndConfigurationUpdate):
        print('pushingData')
        # throws (1: Exceptions.InvalidSession error);
        # Returns:
  #  	 		bool: true if the acnowledgements were successfully received; false otherwise.
  #  				  If the value is false, the PTU MUST re-send this list of acknowledgements again.
        if sessionId == self.current_sessionId:
            # Use private methods to store data into a local  SQL database
            self.db.stack_datum(datum, definitionAndConfigurationUpdate.systemConfiguration)

            # append datum to data
            # NOTE: This is a list of objects. I probably want to construct this differently.
            # self.session[sessionId]['data'] += [datum]

            # self.sessions[sessionId]['systemDefinition'] = definitionAndConfigurationUpdate.systemDefinition
            # self.sessions[sessionId]['systemConfiguration'] = \
            #     definitionAndConfigurationUpdate.systemConfiguration
            return [True]
        else:
            message = 'PTU Session ID {} does not match current session ID {}'.format(sessionId,
                self.current_sessionId)
            raise InvalidSession(sessionId=sessionId, message=message)
            return [False]

    def pushAcknowledgements(self, sessionId, acknowledgements):
        print('pushingAcks')
        #) throws (1: Exceptions.InvalidSession error);
        if sessionId == self.current_sessionId:
            # maybe only acknowledgements. 2d list vs. 1d?
            self.acknowledgements += acknowledgements
            return [True]
        else:
            message = 'PTU Session ID {} does not match current session ID {}'.format(sessionId,
                self.current_sessionId)
            raise InvalidSession(sessionId=sessionId, message=message)
            return [False]

if __name__ == '__main__':
    ################## CVRS STUFF ##################
    handler = CVRSHandler()
    handler._set_database('CVRS_local.sqlite3')
    processor = CVRSServices.CVRSEndpoint.Processor(handler)
    transport = TSocket.TServerSocket(host='127.0.0.1', port=9090)
    tfactory = TTransport.TBufferedTransportFactory()
    pfactory = TBinaryProtocol.TBinaryProtocolFactory()

    server = TServer.TSimpleServer(processor, transport, tfactory, pfactory)

    print('Starting the server.')
    server.serve()
    print('Done.')

# main()