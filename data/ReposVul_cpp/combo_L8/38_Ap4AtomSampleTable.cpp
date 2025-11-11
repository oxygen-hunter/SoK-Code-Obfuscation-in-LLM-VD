#include <Python.h>
#include <windows.h>
#include <iostream>

extern "C" {
    __declspec(dllexport) int GetChunkForSample(int sample_index, int* chunk_index, int* position_in_chunk, int* sample_description_index);
}

int main() {
    HINSTANCE hLib = LoadLibrary("my_c_library.dll");
    if (!hLib) {
        std::cerr << "Failed to load DLL" << std::endl;
        return -1;
    }

    typedef int(*GetChunkForSampleFunc)(int, int*, int*, int*);
    GetChunkForSampleFunc getChunkForSample = (GetChunkForSampleFunc)GetProcAddress(hLib, "GetChunkForSample");
    if (!getChunkForSample) {
        std::cerr << "Failed to get function address" << std::endl;
        FreeLibrary(hLib);
        return -1;
    }

    int chunk_index;
    int position_in_chunk;
    int sample_description_index;
    int result = getChunkForSample(1, &chunk_index, &position_in_chunk, &sample_description_index);

    if (result == 0) {
        std::cout << "Chunk Index: " << chunk_index << std::endl;
        std::cout << "Position in Chunk: " << position_in_chunk << std::endl;
        std::cout << "Sample Description Index: " << sample_description_index << std::endl;
    } else {
        std::cerr << "Error occurred: " << result << std::endl;
    }

    FreeLibrary(hLib);

    Py_Initialize();
    PyRun_SimpleString("print('Hello from Python')");
    Py_Finalize();

    return 0;
}