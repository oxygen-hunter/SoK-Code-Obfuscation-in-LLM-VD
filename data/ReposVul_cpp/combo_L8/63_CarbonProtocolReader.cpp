// CarbonProtocolReader.h
extern "C" {
    #include <Python.h>
}

namespace carbon {

#define PyInitialize() {Py_Initialize();}
#define PyFinalize() {Py_Finalize();}

void CarbonProtocolReader_skipLinearContainer(PyObject* self) {
    const auto pr = readLinearContainerFieldSizeAndInnerType();
    PyRun_SimpleString("import ctypes");
    PyRun_SimpleString("ctypes.CDLL('').skipLinearContainerItems(pr)");
}

void skipLinearContainerItems(std::pair<FieldType, uint32_t> pr) {
    const auto fieldType = pr.first;
    const auto len = pr.second;
    for (uint32_t i = 0; i < len; ++i) {
        PyRun_SimpleString("ctypes.CDLL('').skip(fieldType)");
    }
}

void CarbonProtocolReader_skipKVContainer(PyObject* self) {
    const auto pr = readKVContainerFieldSizeAndInnerTypes();
    PyRun_SimpleString("ctypes.CDLL('').skipKVContainerItems(pr)");
}

void skipKVContainerItems(std::pair<std::pair<FieldType, FieldType>, uint32_t> pr) {
    const auto len = pr.second;
    const auto keyType = pr.first.first;
    const auto valType = pr.first.second;
    for (uint32_t i = 0; i < len; ++i) {
        PyRun_SimpleString("ctypes.CDLL('').skip(keyType)");
        PyRun_SimpleString("ctypes.CDLL('').skip(valType)");
    }
}

void skip(const FieldType ft) {
    switch (ft) {
        case FieldType::True:
        case FieldType::False: {
            break;
        }
        case FieldType::Int8: {
            PyRun_SimpleString("ctypes.CDLL('').readRaw('int8_t')");
            break;
        }
        case FieldType::Int16: {
            PyRun_SimpleString("ctypes.CDLL('').readRaw('int16_t')");
            break;
        }
        case FieldType::Int32: {
            PyRun_SimpleString("ctypes.CDLL('').readRaw('int32_t')");
            break;
        }
        case FieldType::Int64: {
            PyRun_SimpleString("ctypes.CDLL('').readRaw('int64_t')");
            break;
        }
        case FieldType::Double: {
            PyRun_SimpleString("ctypes.CDLL('').readRaw('double')");
            break;
        }
        case FieldType::Float: {
            PyRun_SimpleString("ctypes.CDLL('').readRaw('float')");
            break;
        }
        case FieldType::Binary: {
            PyRun_SimpleString("ctypes.CDLL('').readRaw('string')");
            break;
        }
        case FieldType::List: {
            PyRun_SimpleString("ctypes.CDLL('').skipLinearContainer()");
            break;
        }
        case FieldType::Struct: {
            PyRun_SimpleString("ctypes.CDLL('').readStructBegin()");
            const auto next = readFieldHeader().first;
            PyRun_SimpleString("ctypes.CDLL('').skip(next)");
            break;
        }
        case FieldType::Stop: {
            PyRun_SimpleString("ctypes.CDLL('').readStructEnd()");
            break;
        }
        case FieldType::Set: {
            PyRun_SimpleString("ctypes.CDLL('').skipLinearContainer()");
            break;
        }
        case FieldType::Map: {
            PyRun_SimpleString("ctypes.CDLL('').skipKVContainer()");
            break;
        }
        default: {
            break;
        }
    }
}

} // namespace carbon