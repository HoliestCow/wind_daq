
import ctypes
import numpy as np

# _lib = ctypes.CDLL('/home/holiestcow/Documents/winds/thrift/wind_daq/libs/ptu/build/lib.linux-x86_64-3.5/caenReadoutLib.cpython-35m-x86_64-linux-gnu.so')
_lib = ctypes.CDLL('/home/holiestcow/Documents/winds/thrift/wind_daq/libs/digites/libproject.so')
# Supply the shape of the  numpy array for  the arg types. There shouldn't have any results.

_doublepp = np.ctypeslib.ndpointer(dtype=np.uintp, ndim=1, flags='C')
_lib.measurement_spool.argtypes = [np.ctypeslib.ndpointer(ctypes.c_int, flags="C_CONTIGUOUS")]
_lib.readout_histograms.argtypes = [_doublepp]

def get_histograms(container):
    global _lib
    containerpp = (container.__array_interface__['data'][0] +
                 np.arange(container.shape[0])*container.strides[0]).astype(np.uintp)
    _lib.readout_histograms(containerpp)
    return


def measurement_spool(state):
    # input: state should already be a c pointer
    # state should be passed out so it can be controlled outside the python call to the wrapper.
    # It's intended that the state is used sort of like a reference "&".
    # State = 0 - no command
    # state = 1 - start data acquisition
    # state = 2 - stop data Acquisition
    # state = 3 - clean up and jump out of the measurement_spool. Free up the thread.
    global _lib
    _lib.measurement_spool(state)
    return state
