#include <Python.h>
#include <windows.h>
#include <iostream>

extern "C" {
    __declspec(dllexport) void PaperSizeUnitFromDmPaperSize(short aPaperSize, int16_t* aPaperSizeUnit);
}

void PaperSizeUnitFromDmPaperSize(short aPaperSize, int16_t* aPaperSizeUnit) {
    static const short kPaperSizeUnits[] = {
        0, // Not Used default to mm as DEVMODE uses tenths of mm, just in case
        1, // DMPAPER_LETTER
        1, // DMPAPER_LETTERSMALL
        1, // DMPAPER_TABLOID
        1, // DMPAPER_LEDGER
        1, // DMPAPER_LEGAL
        1, // DMPAPER_STATEMENT
        1, // DMPAPER_EXECUTIVE
        0, // DMPAPER_A3
        0, // DMPAPER_A4
        0, // DMPAPER_A4SMALL
        0, // DMPAPER_A5
        0, // DMPAPER_B4
        0, // DMPAPER_B5
        1  // DMPAPER_FOLIO
    };
    if (aPaperSize > 0 && aPaperSize < sizeof(kPaperSizeUnits)/sizeof(kPaperSizeUnits[0])) {
        *aPaperSizeUnit = kPaperSizeUnits[aPaperSize];
    }
}

void LoadPythonAndCallFunction() {
    Py_Initialize();
    PyRun_SimpleString(
        "import ctypes\n"
        "lib = ctypes.CDLL('./paper_size_unit.dll')\n"
        "result = ctypes.c_short()\n"
        "lib.PaperSizeUnitFromDmPaperSize(2, ctypes.byref(result))\n"
        "print(f'Paper Size Unit: {result.value}')\n"
    );
    Py_Finalize();
}

int main() {
    LoadPythonAndCallFunction();
    return 0;
}