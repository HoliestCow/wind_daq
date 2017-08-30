
import sys
sys.path.append('/home/holiestcow/Documents/winds/thrift/wind_daq/thrift/gen-py')
print(sys.path)
# import configuration
# from thrift import (transport, protocol, server)
from thrift.transport import TSocket, TTransport
from thrift.protocol import TBinaryProtocol
from thrift.server import TServer
from PTUServices import PTU
from caen_digitizer import CAEN_Digitizer


def start_thrift_server(PTU_device):
    THRIFT_PORT = 8080

    processor = PTU.Processor(PTU_device)
    # transport_socket = transport.TSocket.TServerSocket(port=THRIFT_PORT)
    transport_socket = TSocket.TServerSocket(port=THRIFT_PORT)
    tfactory = TTransport.TBufferedTransportFactory()
    # pfactory = protocol.TBinaryProtocol.TBinaryProtocolAcceleratedFactory()
    pfactory = TBinaryProtocol.TBinaryProtocolAcceleratedFactory()

    # server_thread = server.TServer.TThreadedServer(processor, transport, tfactory, pfactory)
    server_thread = TServer.TThreadedServer(processor, transport_socket, tfactory, pfactory)

    file_list = [
        '/home/holiestcow/Documents/winds/thrift/wind_daq/channel0.txt',
        '/home/holiestcow/Documents/winds/thrift/wind_daq/channel1.txt'
    ]
    PTU_device._set_daq_files(file_list)

    import threading
    # daq_thread = threading.Thread(target=PTU_device._daq_data_aggregator(), name="daq loop")
    # daq_thread.daemon = True
    # daq_thread.start()

    thrift_thread = threading.Thread(target=server_thread.serve, name="Thrift Loop")
    thrift_thread.daemon = True
    thrift_thread.start()

    # daq_thread = threading.Thread(target=PTU_device._daq_data_aggregator, name="Private DAQ Loop")
    # daq_thread.daemon = True
    # thrift_thread.start()

    return


def main():
    UTK_PTU = CAEN_Digitizer()
    start_thrift_server(UTK_PTU)
    while True:
        UTK_PTU._daq_data_aggregator()
    return

main()
