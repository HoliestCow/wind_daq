
import ctypes
import numpy as np

_lib = ctypes.CDLL('./build/lib.linux-x86_64-3.5/funclib.cpython-35m-x86_64-linux-gnu.so')
_lib.running_process.argtypes = []
# _lib.running_process.argtypes = [np.ctypeslib.ndpointer(ctypes.c_uint32, flags="C_CONTIGUOUS"),
#                                  np.ctypeslib.ndpointer(ctypes.c_uint32, flags="C_CONTIGUOUS"),
#                                  ctypes.c_size_t]
_lib.running_process.argtypes = [np.ctypeslib.ndpointer(ctypes.c_int32, flags="C_CONTIGUOUS"),
                                 np.ctypeslib.ndpointer(ctypes.c_int32, flags="C_CONTIGUOUS"),
                                 ctypes.c_size_t]
# lib.callMe.restype = ndpointer(dtype=ctypes.c_int, shape=(4, 3))


def running_process(state, data, size):
    _lib.running_process(state, data, size)
    return
