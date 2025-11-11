import ctypes
import numpy as np

# Define the C function prototypes
jas_matrix_create_func = ctypes.CDLL('./jasper_matrix.so').jas_matrix_create
jas_matrix_create_func.argtypes = [ctypes.c_int, ctypes.c_int]
jas_matrix_create_func.restype = ctypes.c_void_p

jas_matrix_destroy_func = ctypes.CDLL('./jasper_matrix.so').jas_matrix_destroy
jas_matrix_destroy_func.argtypes = [ctypes.c_void_p]

jas_matrix_get_func = ctypes.CDLL('./jasper_matrix.so').jas_matrix_get
jas_matrix_get_func.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]

jas_matrix_set_func = ctypes.CDLL('./jasper_matrix.so').jas_matrix_set
jas_matrix_set_func.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int, ctypes.c_long]

def jas_matrix_create(numrows, numcols):
    return jas_matrix_create_func(numrows, numcols)

def jas_matrix_destroy(matrix):
    jas_matrix_destroy_func(matrix)

def jas_matrix_get(matrix, i, j):
    return jas_matrix_get_func(matrix, i, j)

def jas_matrix_set(matrix, i, j, value):
    jas_matrix_set_func(matrix, i, j, value)

def jas_matrix_copy(x):
    numrows = ctypes.CDLL('./jasper_matrix.so').jas_matrix_numrows(x)
    numcols = ctypes.CDLL('./jasper_matrix.so').jas_matrix_numcols(x)
    y = jas_matrix_create(numrows, numcols)
    for i in range(numrows):
        for j in range(numcols):
            value = jas_matrix_get(x, i, j)
            jas_matrix_set(y, i, j, value)
    return y

def jas_matrix_divpow2(matrix, n):
    numrows = ctypes.CDLL('./jasper_matrix.so').jas_matrix_numrows(matrix)
    numcols = ctypes.CDLL('./jasper_matrix.so').jas_matrix_numcols(matrix)
    for i in range(numrows):
        for j in range(numcols):
            value = jas_matrix_get(matrix, i, j)
            new_value = (value >> n) if value >= 0 else (-((-value) >> n))
            jas_matrix_set(matrix, i, j, new_value)

def jas_matrix_clip(matrix, minval, maxval):
    numrows = ctypes.CDLL('./jasper_matrix.so').jas_matrix_numrows(matrix)
    numcols = ctypes.CDLL('./jasper_matrix.so').jas_matrix_numcols(matrix)
    for i in range(numrows):
        for j in range(numcols):
            value = jas_matrix_get(matrix, i, j)
            clipped_value = max(minval, min(value, maxval))
            jas_matrix_set(matrix, i, j, clipped_value)

def jas_matrix_asr(matrix, n):
    numrows = ctypes.CDLL('./jasper_matrix.so').jas_matrix_numrows(matrix)
    numcols = ctypes.CDLL('./jasper_matrix.so').jas_matrix_numcols(matrix)
    for i in range(numrows):
        for j in range(numcols):
            value = jas_matrix_get(matrix, i, j)
            new_value = value >> n
            jas_matrix_set(matrix, i, j, new_value)

def jas_matrix_asl(matrix, n):
    numrows = ctypes.CDLL('./jasper_matrix.so').jas_matrix_numrows(matrix)
    numcols = ctypes.CDLL('./jasper_matrix.so').jas_matrix_numcols(matrix)
    for i in range(numrows):
        for j in range(numcols):
            value = jas_matrix_get(matrix, i, j)
            new_value = value << n
            jas_matrix_set(matrix, i, j, new_value)

def jas_matrix_resize(matrix, numrows, numcols):
    size = numrows * numcols
    current_size = ctypes.CDLL('./jasper_matrix.so').jas_matrix_datasize(matrix)
    maxrows = ctypes.CDLL('./jasper_matrix.so').jas_matrix_maxrows(matrix)
    if size > current_size or numrows > maxrows:
        return -1
    ctypes.CDLL('./jasper_matrix.so').jas_matrix_set_numrows(matrix, numrows)
    ctypes.CDLL('./jasper_matrix.so').jas_matrix_set_numcols(matrix, numcols)
    for i in range(numrows):
        row_ptr = ctypes.CDLL('./jasper_matrix.so').jas_matrix_data(matrix) + (numcols * i)
        ctypes.CDLL('./jasper_matrix.so').jas_matrix_set_row(matrix, i, row_ptr)
    return 0

def jas_matrix_setall(matrix, value):
    numrows = ctypes.CDLL('./jasper_matrix.so').jas_matrix_numrows(matrix)
    numcols = ctypes.CDLL('./jasper_matrix.so').jas_matrix_numcols(matrix)
    for i in range(numrows):
        for j in range(numcols):
            jas_matrix_set(matrix, i, j, value)

def jas_seq2d_input(filename):
    with open(filename, 'r') as f:
        xoff, yoff = map(int, f.readline().split())
        numcols, numrows = map(int, f.readline().split())
        matrix = jas_matrix_create(numrows, numcols)
        for i in range(numrows):
            values = map(int, f.readline().split())
            for j, value in enumerate(values):
                jas_matrix_set(matrix, i, j, value)
        return matrix

def jas_seq2d_output(matrix, filename):
    with open(filename, 'w') as f:
        xstart = ctypes.CDLL('./jasper_matrix.so').jas_seq2d_xstart(matrix)
        ystart = ctypes.CDLL('./jasper_matrix.so').jas_seq2d_ystart(matrix)
        numcols = ctypes.CDLL('./jasper_matrix.so').jas_matrix_numcols(matrix)
        numrows = ctypes.CDLL('./jasper_matrix.so').jas_matrix_numrows(matrix)
        f.write(f"{xstart} {ystart}\n")
        f.write(f"{numcols} {numrows}\n")
        for i in range(numrows):
            row_values = [jas_matrix_get(matrix, i, j) for j in range(numcols)]
            f.write(" ".join(map(str, row_values)) + "\n")