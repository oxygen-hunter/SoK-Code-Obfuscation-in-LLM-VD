import ctypes
from ctypes import c_char_p, c_int, POINTER, Structure

# Load the C library
uriparser = ctypes.CDLL('./liburiparser.so')

# Define the necessary C structures
class QueryList(Structure):
    pass

QueryList._fields_ = [('key', c_char_p), ('value', c_char_p), ('next', POINTER(QueryList))]

# Define the functions from the C library
ComposeQueryCharsRequiredEx = uriparser.ComposeQueryCharsRequiredEx
ComposeQueryCharsRequiredEx.argtypes = [POINTER(QueryList), POINTER(c_int), ctypes.c_bool, ctypes.c_bool]
ComposeQueryCharsRequiredEx.restype = c_int

ComposeQueryEngine = uriparser.ComposeQueryEngine
ComposeQueryEngine.argtypes = [c_char_p, POINTER(QueryList), c_int, POINTER(c_int), POINTER(c_int), ctypes.c_bool, ctypes.c_bool]
ComposeQueryEngine.restype = c_int

# Python wrapper functions
def compose_query_chars_required(query_list):
    chars_required = c_int()
    res = ComposeQueryCharsRequiredEx(query_list, ctypes.byref(chars_required), True, True)
    if res != 0:
        raise ValueError("Error in ComposeQueryCharsRequiredEx")
    return chars_required.value

def compose_query_engine(dest, query_list, max_chars):
    chars_written = c_int()
    res = ComposeQueryEngine(dest, query_list, max_chars, ctypes.byref(chars_written), None, True, True)
    if res != 0:
        raise ValueError("Error in ComposeQueryEngine")
    return chars_written.value

# Example usage in Python
query_list = QueryList()
query_list.key = b"key"
query_list.value = b"value"
query_list.next = None

chars_required = compose_query_chars_required(ctypes.byref(query_list))
print(f"Characters required: {chars_required}")

dest = ctypes.create_string_buffer(chars_required + 1)
chars_written = compose_query_engine(dest, ctypes.byref(query_list), chars_required + 1)
print(f"Query: {dest.value.decode('utf-8')}, Characters written: {chars_written}")