import sys
sys.path.append('/home/holiestcow/Documents/winds/thrift/wind_daq/thrift/gen-py')
print(sys.path)
# import configuration
from thrift import (transport, protocol)
from PTUServices import PTU
from caen_digitizer import CAEN_Digitizer


def start_thrift_server():
    THRIFT_PORT = 9090
    processor = PTU.Processor(CAEN_Digitizer)
    transport = transport.TSocket.TServerSocket(port=THRIFT_PORT)
    tfactory = transport.TTransport.TBufferedTransportFactory()
    pfactory = protocol.TBinaryProtocol.TBinaryProtocolAcceleratedFactory()

    server = thrift.server.TServer.TThreadedServer(processor, transport, tfactory, pfactory)

    import threading
    thrift_thread = threading.Thread(target=server.serve, name="Thrift Loop")
    thrift_thread.daemon = True
    thrift_thread.start()
    return

def main():
    start_thrift_server()
    return

main()
