
import ctypes
import numpy as np

# _lib = ctypes.CDLL('./build/lib.linux-x86_64-3.5/funclib.cpython-35m-x86_64-linux-gnu.so')
# _lib.spool_measurement.argtypes = (ctypes.POINTER(ctypes.c_int))

_lib = ctypes.CDLL('/home/holiestcow/Documents/winds/thrift/wind_daq/libs/ptu/build/lib.linux-x86_64-3.5/caenReadoutLib.cpython-35m-x86_64-linux-gnu.so')
# Supply the shape of the  numpy array for  the arg types. There shouldn't have any results.

_doublepp = np.ctypeslib.ndpointer(dtype=np.uintp, ndim=1, flags='C')

_lib.measurement_spool.argtypes = [np.ctypeslib.ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"),
                                   _doublepp,
                                   _doublepp,
                                   ctypes.c_size_t]



def measurement_spool(state, short_data, long_data, size, shape):
    # input: state should already be a c pointer
    # State = 0 - no command
    # state = 1 - start data acquisition
    # state = 2 - stop data Acquisition
    # state = 3 - clean up and jump out of the measurement_spool. Free up the thread.
    global _lib
    # _lib.measurement_spool.argtypes = [np.ctypeslib.ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"),
    #                                    np.ctypeslib.ndpointer(
    #                                        ctypes.c_uint32, ndim=2, shape=shape,
    #                                        flags="C_CONTIGUOUS"),
    #                                    np.ctypeslib.ndpointer(
    #                                        ctypes.c_uint32, ndim=2, shape=shape,
    #                                        flags="C_CONTIGUOUS"),
    #                                    ctypes.c_size_t]
    shortpp = (short_data.__array_interface__['data'][0] +
               np.arange(short_data.shape[0])*short_data.strides[0]).astype(np.uintp)
    longpp = (long_data.__array_interface__['data'][0] +
              np.arange(long_data.shape[0])*long_data.strides[0]).astype(np.uintp)

    _lib.measurement_spool(state, shortpp, longpp, shape[1])
    return
