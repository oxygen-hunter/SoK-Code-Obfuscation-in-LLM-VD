```python
import ctypes
from ctypes import CDLL, c_void_p, c_int, c_uint, POINTER, Structure, c_char_p

# Load the shared library
lib = CDLL('./librds.so')  # Assume the original C code is compiled into a shared library named librds.so

# Define structures and functions using ctypes
class RDS_IB_Device(Structure):
    _fields_ = [('refcount', c_int), ('free_work', c_void_p)]  # Simplified for demonstration

class IB_Device(Structure):
    _fields_ = [('node_type', c_int), ('name', c_char_p)]

# Function prototypes
lib.rds_ib_dev_put.argtypes = [POINTER(RDS_IB_Device)]
lib.rds_ib_dev_put.restype = None

lib.rds_ib_add_one.argtypes = [POINTER(IB_Device)]
lib.rds_ib_add_one.restype = None

def rds_ib_add_one(device):
    if device.contents.node_type != RDMA_NODE_IB_CA:
        return

    dev_attr = ctypes.create_string_buffer(ctypes.sizeof(c_int))
    if not dev_attr:
        return

    if lib.ib_query_device(device, dev_attr):
        print(f"Query device failed for {device.contents.name}")
        free_attr(dev_attr)
        return

    rds_ibdev = ctypes.create_string_buffer(ctypes.sizeof(RDS_IB_Device))
    if not rds_ibdev:
        free_attr(dev_attr)
        return

    rds_ibdev_ptr = ctypes.cast(rds_ibdev, POINTER(RDS_IB_Device))
    lib.rds_ib_dev_put(rds_ibdev_ptr)
    free_attr(dev_attr)

def free_attr(attr):
    lib.free(attr)

RDMA_NODE_IB_CA = 1  # Example constant
```

Note: The above code assumes that the original C code has been compiled into a shared library named `librds.so`. The functions and structures are simplified and may need to be adjusted according to the actual C library's definitions and usage.