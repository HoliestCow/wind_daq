# from wind_daq.core.configuration
import wind_daq.core.configuration
import thrift
#from wind_daq.core.caen_digitizer import CAEN_Digitizer
#from caen_digitizer import CAEN_Digitizer
from PTUServices import PTU
from wind_daq.core.caen_digitzer import CAEN_Digitizer

blah = CAEN_Digitizer
def start_thrift_server():
    processor = PTU.Processor(CAEN_Digitizer)
    transport = thrift.transport.TSocket.TServerSocket(port=THRIFT_PORT)
    tfactory = thrift.transport.TTransport.TBufferedTransportFactory()
    pfactory = thrift.protocol.TBinaryProtocol.TBinaryProtocolAcceleratedFactory()

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
